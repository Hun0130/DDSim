// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file HelloWorldSimulator.cpp
 * 
 * 간단한 DDS 퍼블리셔/서브스크라이버 시뮬레이터
 * 1초마다 메시지를 송수신하는 기본 기능만 포함
 */

#include "HelloWorldPubSubTypes.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <sstream>
#include <deque>
#include <unordered_map>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <cstring> // for memcpy

// UDPTransportInterface에서 구현된 데이터 구조체 선언
namespace eprosima {
namespace fastdds {
namespace rtps {
    struct SerializedOutputData {
        std::vector<uint8_t> data;
        std::string destination;
        
        void print() const {
            std::stringstream ss;
            ss << "SerializedOutputData to " << destination << " [" << data.size() << " bytes]: " << std::hex;
            for (size_t i = 0; i < data.size() && i < 64; i++) {
                ss << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
                if ((i + 1) % 16 == 0) ss << std::endl;
            }
            if (data.size() > 64) ss << "...";
            if (data.size() % 16 != 0) ss << std::endl;
            ss << std::dec;
            std::cout << ss.str() << std::endl;
        }
    };
    
    // UDPTransportInterface에서 구현된 함수 선언
    extern thread_local SerializedOutputData last_serialized_data;
    SerializedOutputData& get_last_serialized_data();
    
    // 메시지 히스토리 관련 함수 선언 
    size_t get_serialized_history_size();
    SerializedOutputData get_serialized_history_at(size_t index);
    std::vector<SerializedOutputData> get_serialized_history();
    void clear_serialized_history();
}}}

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

// DDS 발신 메시지 큐 및 관리 함수들
std::mutex g_dds_message_mutex;
std::deque<SerializedOutputData> g_dds_message_history;
const size_t MAX_DDS_MESSAGE_HISTORY = 1000; // 최대 1000개 메시지 저장

// DDS 메시지를 큐에 추가
void save_dds_message(const SerializedOutputData& data) {
    std::lock_guard<std::mutex> lock(g_dds_message_mutex);
    
    // 큐 크기 제한
    if (g_dds_message_history.size() >= MAX_DDS_MESSAGE_HISTORY) {
        g_dds_message_history.pop_front();
    }
    
    // 데이터 추가
    g_dds_message_history.push_back(data);
    
    // 메시지 저장 로그 출력
    std::cout << "DDS 메시지 캡처 [#" << g_dds_message_history.size() << "] 대상: " << data.destination 
              << " 크기: " << data.data.size() << " 바이트";
    
    // RTPS 메시지인지 확인
    if (data.data.size() >= 4 && 
        data.data[0] == 'R' && data.data[1] == 'T' && 
        data.data[2] == 'P' && data.data[3] == 'S') {
        std::cout << " (RTPS)";
    }
    std::cout << std::endl;
}

// 저장된 DDS 메시지의 개수 반환
size_t get_dds_message_count() {
    std::lock_guard<std::mutex> lock(g_dds_message_mutex);
    return g_dds_message_history.size();
}

// 특정 인덱스의 DDS 메시지 반환
SerializedOutputData get_dds_message_at(size_t index) {
    std::lock_guard<std::mutex> lock(g_dds_message_mutex);
    if (index < g_dds_message_history.size()) {
        return g_dds_message_history[index];
    }
    return SerializedOutputData();
}

// 모든 DDS 메시지를 벡터로 반환
std::vector<SerializedOutputData> get_all_dds_messages() {
    std::lock_guard<std::mutex> lock(g_dds_message_mutex);
    return std::vector<SerializedOutputData>(g_dds_message_history.begin(), g_dds_message_history.end());
}

// DDS 메시지 히스토리 초기화
void clear_dds_messages() {
    std::lock_guard<std::mutex> lock(g_dds_message_mutex);
    g_dds_message_history.clear();
    std::cout << "DDS 메시지 히스토리 초기화 완료" << std::endl;
}

// DDS 메시지 히스토리 요약 출력
void print_dds_message_summary(bool detail = false) {
    std::lock_guard<std::mutex> lock(g_dds_message_mutex);
    std::cout << "===== DDS 메시지 히스토리 요약 =====" << std::endl;
    std::cout << "총 캡처된 메시지 수: " << g_dds_message_history.size() << std::endl;
    
    if (detail && !g_dds_message_history.empty()) {
        size_t rtps_count = 0;
        size_t total_bytes = 0;
        std::unordered_map<std::string, size_t> destinations;
        
        for (const auto& msg : g_dds_message_history) {
            total_bytes += msg.data.size();
            destinations[msg.destination]++;
            
            if (msg.data.size() >= 4 && 
                msg.data[0] == 'R' && msg.data[1] == 'T' && 
                msg.data[2] == 'P' && msg.data[3] == 'S') {
                rtps_count++;
            }
        }
        
        std::cout << "RTPS 메시지 수: " << rtps_count << std::endl;
        std::cout << "총 데이터 크기: " << total_bytes << " 바이트" << std::endl;
        std::cout << "목적지별 메시지 수:" << std::endl;
        for (const auto& dest : destinations) {
            std::cout << "  - " << dest.first << ": " << dest.second << "개" << std::endl;
        }
    }
    
    std::cout << "=================================" << std::endl;
}

// 마지막 DDS 메시지 모니터링 스레드
std::atomic<bool> g_monitoring_active(false);
std::thread g_monitoring_thread;

// DDS 메시지 모니터링 시작
void start_dds_message_monitoring() {
    if (g_monitoring_active.load()) {
        std::cout << "DDS 메시지 모니터링이 이미 활성화되어 있습니다." << std::endl;
        return;
    }
    
    g_monitoring_active.store(true);
    clear_dds_messages();
    
    g_monitoring_thread = std::thread([]() {
        SerializedOutputData last_data;
        
        std::cout << "DDS 메시지 모니터링 시작" << std::endl;
        
        while (g_monitoring_active.load()) {
            SerializedOutputData current_data = get_last_serialized_data();
            
            // 새 데이터가 있고 이전 데이터와 다른 경우에만 저장
            if (!current_data.data.empty() && 
                (last_data.data.empty() || 
                 current_data.data != last_data.data || 
                 current_data.destination != last_data.destination)) {
                
                save_dds_message(current_data);
                last_data = current_data;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        std::cout << "DDS 메시지 모니터링 종료" << std::endl;
    });
}

// DDS 메시지 모니터링 중지
void stop_dds_message_monitoring() {
    if (!g_monitoring_active.load()) {
        std::cout << "DDS 메시지 모니터링이 이미 비활성화되어 있습니다." << std::endl;
        return;
    }
    
    g_monitoring_active.store(false);
    
    if (g_monitoring_thread.joinable()) {
        g_monitoring_thread.join();
    }
    
    print_dds_message_summary(true);
}

// DDS 데이터 주입 함수 - FastDDS로 직접 데이터 전달
void inject_dds_data(const std::vector<uint8_t>& data, const std::string& destination = "127.0.0.1:7412") {
    if (data.empty()) {
        std::cerr << "주입할 데이터가 비어 있습니다" << std::endl;
        return;
    }
    
    // 마지막 직렬화 데이터 설정 - 이후 UDPChannelResource::Receive에서 이 데이터를 사용
    SerializedOutputData& serialized_data = get_last_serialized_data();
    serialized_data.data = data;
    serialized_data.destination = destination;
    
    std::cout << "데이터 주입 완료 (" << data.size() << " 바이트)" << std::endl;
}

// 모의 네트워크를 위한 글로벌 전달 메커니즘
struct SerializedData {
    std::vector<uint8_t> data;
    std::string destination;
};

// 글로벌 데이터 큐와 뮤텍스
std::mutex g_data_mutex;
std::vector<SerializedData> g_data_queue;

// 데이터를 모의 네트워크 큐에 추가하는 함수
void add_to_mock_network(const SerializedData& data) {
    std::lock_guard<std::mutex> lock(g_data_mutex);
    g_data_queue.push_back(data);
    std::cout << "Added " << data.data.size() << " bytes to mock network queue (destination: " 
              << data.destination << ")" << std::endl;
}

// 모의 네트워크 큐에서 데이터를 가져오는 함수
bool get_from_mock_network(SerializedData& data) {
    std::lock_guard<std::mutex> lock(g_data_mutex);
    if (g_data_queue.empty()) {
        return false;
    }
    data = g_data_queue.front();
    g_data_queue.erase(g_data_queue.begin());
    return true;
}

// Subscriber 클래스 정의
class HelloWorldSubscriber
{
private:
    DomainParticipant* participant_;
    Subscriber* subscriber_;
    DataReader* reader_;
    Topic* topic_;
    TypeSupport type_;
    
    class SubListener : public DataReaderListener
    {
    public:
        SubListener() : samples_(0) {}
        
        ~SubListener() override {}
        
        void on_subscription_matched(
                DataReader*,
                const SubscriptionMatchedStatus& info) override
        {
            // 필요없는 출력 제거
        }
        
        void on_data_available(DataReader* reader) override
        {
            SampleInfo info;
            if (reader->take_next_sample(&hello_, &info) == RETCODE_OK)
            {
                if (info.valid_data)
                {
                    samples_++;
                    std::cout << "데이터 수신: #" << samples_ << " (인덱스: " << hello_.index() << ")" << std::endl;
                }
            }
        }
        
        HelloWorld hello_;
        std::atomic_int samples_;
    } listener_;
    
public:
    HelloWorldSubscriber()
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , type_(new HelloWorldPubSubType())
    {
    }
    
    virtual ~HelloWorldSubscriber()
    {
        if (reader_ != nullptr) subscriber_->delete_datareader(reader_);
        if (topic_ != nullptr) participant_->delete_topic(topic_);
        if (subscriber_ != nullptr) participant_->delete_subscriber(subscriber_);
        if (participant_ != nullptr) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
    
    bool init()
    {
        // 참여자 생성
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
        if (participant_ == nullptr) return false;
        
        // 타입 등록
        type_.register_type(participant_);
        
        // 토픽 생성
        topic_ = participant_->create_topic("HelloWorldTopic", "HelloWorld", TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr) return false;
        
        // 서브스크라이버 생성
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (subscriber_ == nullptr) return false;
        
        // 데이터 리더 생성
        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);
        if (reader_ == nullptr) return false;
        
        return true;
    }
    
    void run(uint32_t samples)
    {
        std::cout << "구독자 시작 - " << samples << "개 메시지 대기 중..." << std::endl;
        while (listener_.samples_ < samples)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "구독자 종료 - 총 " << listener_.samples_ << "개 메시지 수신 완료" << std::endl;
    }

    // 참여자 얻기 메서드
    DomainParticipant* get_participant() {
        return participant_;
    }
    
    // 수신된 샘플 수 반환
    int get_received_samples() const {
        return listener_.samples_;
    }
};

// Publisher 클래스 수정
class HelloWorldPublisher
{
private:
    HelloWorld hello_;
    DomainParticipant* participant_;
    Publisher* publisher_;
    Topic* topic_;
    DataWriter* writer_;
    TypeSupport type_;
    SerializedOutputData last_published_data_; // 마지막으로 발행된 데이터 저장
    
    class PubListener : public DataWriterListener
    {
    public:
        PubListener() : matched_(0) {}
        
        ~PubListener() override {}
        
        void on_publication_matched(
                DataWriter*,
                const PublicationMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
            {
                matched_ = info.total_count;
                std::cout << "Publisher matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                matched_ = info.total_count;
                std::cout << "Publisher unmatched." << std::endl;
            }
        }
        
        std::atomic_int matched_;
    } listener_;
    
public:
    HelloWorldPublisher()
        : participant_(nullptr)
        , publisher_(nullptr)
        , topic_(nullptr)
        , writer_(nullptr)
        , type_(new HelloWorldPubSubType())
        , last_published_data_()
    {
    }
    
    virtual ~HelloWorldPublisher()
    {
        if (writer_ != nullptr) publisher_->delete_datawriter(writer_);
        if (publisher_ != nullptr) participant_->delete_publisher(publisher_);
        if (topic_ != nullptr) participant_->delete_topic(topic_);
        if (participant_ != nullptr) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
    
    bool init()
    {
        hello_.index(0);
        hello_.message("헬로월드");
        
        // 참여자 생성
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
        if (participant_ == nullptr) return false;
        
        // 타입 등록
        type_.register_type(participant_);
        
        // 토픽 생성
        topic_ = participant_->create_topic("HelloWorldTopic", "HelloWorld", TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr) return false;
        
        // 퍼블리셔 생성
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        if (publisher_ == nullptr) return false;
        
        // 데이터 라이터 생성
        writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);
        if (writer_ == nullptr) return false;
        
        // 테스트를 위해 항상 매칭된 상태로 설정
        listener_.matched_ = 1;
        
        return true;
    }
    
    bool publish()
    {
        if (listener_.matched_ > 0)
        {
            hello_.index(hello_.index() + 1);
            
            // 데이터 발행
            writer_->write(&hello_);
            
            // 발행 직후 직렬화된 데이터 저장
            last_published_data_ = get_last_serialized_data();
            
            std::cout << "데이터 발행: #" << hello_.index() << " (크기: " << last_published_data_.data.size() << " 바이트)" << std::endl;
            
            return true;
        }
        return false;
    }
    
    // 마지막으로 발행된 데이터 반환
    const SerializedOutputData& get_last_published_data() const {
        return last_published_data_;
    }
    
    // 마지막으로 발행된 메시지 인덱스 반환
    int get_last_index() const {
        return hello_.index();
    }
    
    void run(uint32_t samples)
    {
        std::cout << "발행자 시작 - " << samples << "개 메시지 전송 예정" << std::endl;
        uint32_t samples_sent = 0;
        while (samples_sent < samples)
        {
            if (publish())
            {
                samples_sent++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        std::cout << "발행자 종료 - 총 " << samples_sent << "개 메시지 전송 완료" << std::endl;
    }
};

// 간단한 시뮬레이터 클래스
class HelloWorldSimulator
{
private:
    uint32_t num_samples_;
    std::shared_ptr<HelloWorldSubscriber> subscriber_;
    bool monitoring_enabled_;
    
public:
    HelloWorldSimulator(uint32_t num_samples = 10)
        : num_samples_(num_samples)
        , subscriber_(nullptr)
        , monitoring_enabled_(false)
    {
    }
    
    void run()
    {
        std::cout << "=== DDS 시뮬레이터 시작 (샘플 수: " << num_samples_ << ") ===" << std::endl;
        
        // 퍼블리셔와 서브스크라이버 생성
        HelloWorldPublisher publisher;
        subscriber_ = std::make_shared<HelloWorldSubscriber>();
        
        // 초기화 확인
        if (!publisher.init() || !subscriber_->init())
        {
            std::cerr << "초기화 실패. 프로그램을 종료합니다." << std::endl;
            return;
        }
        
        // 서브스크라이버 스레드 시작 - 별도 스레드에서 메시지 수신
        std::thread sub_thread([&]() {
            subscriber_->run(num_samples_);
        });
        
        // 퍼블리셔 실행 (메인 스레드)
        publisher.run(num_samples_);
        
        // 서브스크라이버 스레드 종료 대기
        if (sub_thread.joinable())
            sub_thread.join();
        
        std::cout << "=== DDS 시뮬레이터 종료 ===" << std::endl;
    }

    // 데이터 주입 헬퍼 메서드
    void inject_data(const std::vector<uint8_t>& data, const std::string& destination = "127.0.0.1:7412") {
        if (subscriber_) {
            DomainParticipant* participant = subscriber_->get_participant();
            if (participant) {
                inject_dds_data(data, destination);
            } else {
                std::cerr << "참여자 인스턴스를 찾을 수 없습니다" << std::endl;
            }
        } else {
            std::cerr << "구독자 인스턴스를 찾을 수 없습니다" << std::endl;
        }
    }
    
    // DDS 메시지 모니터링 시작
    void start_monitoring() {
        start_dds_message_monitoring();
        monitoring_enabled_ = true;
    }
    
    // DDS 메시지 모니터링 중지
    void stop_monitoring() {
        if (monitoring_enabled_) {
            stop_dds_message_monitoring();
            monitoring_enabled_ = false;
        }
    }
    
    // 소멸자에서 모니터링 중지
    ~HelloWorldSimulator() {
        stop_monitoring();
    }
};

int main(int argc, char** argv)
{
    // 샘플 수 설정 (기본값 10)
    uint32_t samples = 10;
    
    // 인자가 있으면 샘플 수로 사용
    if (argc > 1)
        samples = atoi(argv[1]);
    
    // 시뮬레이터 생성
    HelloWorldSimulator simulator(samples);
    
    // DDS 메시지 모니터링 시작
    simulator.start_monitoring();
    
    // 시뮬레이터 실행
    simulator.run();
    
    // 결과 요약 출력
    print_dds_message_summary(true);
    
    std::cout << "\n=== 데이터 주입 테스트 ===" << std::endl;
    
    // 테스트 데이터 생성
    HelloWorld test_msg;
    test_msg.index(999);
    test_msg.message("직접 주입된 메시지");
    
    // 직렬화 수행
    HelloWorldPubSubType type;
    eprosima::fastdds::rtps::SerializedPayload_t payload(128);
    if (type.serialize(&test_msg, payload, eprosima::fastdds::dds::DataRepresentationId_t::XCDR2_DATA_REPRESENTATION)) {
        // 바이트 벡터로 변환
        std::vector<uint8_t> serialized_data(payload.data, payload.data + payload.length);
        
        std::cout << "테스트 메시지 직렬화 완료 (인덱스: " << test_msg.index() << ")" << std::endl;
        
        // 데이터 주입
        simulator.inject_data(serialized_data, "127.0.0.1:7412");
        
        // 잠시 대기
        std::this_thread::sleep_for(std::chrono::seconds(5));
    } else {
        std::cerr << "메시지 직렬화 실패" << std::endl;
    }
    
    // 최종 결과 요약 출력
    print_dds_message_summary(true);
    
    // 모니터링 종료
    simulator.stop_monitoring();
    
    std::cout << "=== 데이터 주입 테스트 종료 ===" << std::endl;
    
    return 0;
} 