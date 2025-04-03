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
 * 하나의 터미널에서 Publisher와 Subscriber를 동시에 실행하는 시뮬레이터
 */

#include "HelloWorldPubSubTypes.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Time_t.hpp>

using namespace eprosima::fastdds::dds;

// Publisher 클래스 정의
class HelloWorldPublisher
{
private:
    HelloWorld hello_;
    DomainParticipant* participant_;
    Publisher* publisher_;
    Topic* topic_;
    DataWriter* writer_;
    TypeSupport type_;
    std::string participant_name_;

    class PubListener : public DataWriterListener
    {
    public:
        PubListener()
            : matched_(0)
        {
        }

        ~PubListener() override
        {
        }

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
            else
            {
                std::cout << info.current_count_change
                        << " is not a valid value for PublicationMatchedStatus current count change." << std::endl;
            }
        }

        std::atomic_int matched_;
    } listener_;

public:
    HelloWorldPublisher(const std::string& participant_name = "Participant_publisher")
        : participant_(nullptr)
        , publisher_(nullptr)
        , topic_(nullptr)
        , writer_(nullptr)
        , type_(new HelloWorldPubSubType())
        , participant_name_(participant_name)
    {
    }

    virtual ~HelloWorldPublisher()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }
        if (publisher_ != nullptr)
        {
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    //!Initialize the publisher
    bool init()
    {
        hello_.index(0);
        hello_.message("HelloWorld");

        DomainParticipantQos participantQos;
        participantQos.name(participant_name_);
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

        if (participant_ == nullptr)
        {
            return false;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the publications Topic
        topic_ = participant_->create_topic("HelloWorldTopic", "HelloWorld", TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            return false;
        }

        // Create the Publisher
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

        if (publisher_ == nullptr)
        {
            return false;
        }

        // Create the DataWriter
        writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);

        if (writer_ == nullptr)
        {
            return false;
        }
        return true;
    }

    //!Send a publication
    bool publish()
    {
        if (listener_.matched_ > 0)
        {
            hello_.index(hello_.index() + 1);
            
            // DDS 시간 출력 코드 추가
            eprosima::fastdds::rtps::Time_t current_time;
            // 시간 초기화
            eprosima::fastdds::rtps::Time_t::now(current_time);
            
            std::cout << "Current DDS Time - Seconds: " << current_time.seconds() 
                    << " Nanosecs: " << current_time.nanosec() << std::endl;
            
            // 시간 증가
            current_time.increment_time(1, 0);
            
            // 증가된 시간 출력
            std::cout << "After increment_time(1,0) - Seconds: " << current_time.seconds() 
                    << " Nanosecs: " << current_time.nanosec() << std::endl;
            
            writer_->write(&hello_);
            return true;
        }
        return false;
    }

    //!Run the Publisher
    void run(
            uint32_t samples,
            std::atomic_bool& stop_flag)
    {
        uint32_t samples_sent = 0;
        while (samples_sent < samples && !stop_flag.load())
        {
            if (publish())
            {
                samples_sent++;
                std::cout << "Publisher [" << participant_name_ << "] Message: " << hello_.message() 
                         << " with index: " << hello_.index() << " SENT" << std::endl;
            }
            // 실제 시간 사용
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
};

// Subscriber 클래스 정의
class HelloWorldSubscriber
{
private:
    DomainParticipant* participant_;
    Subscriber* subscriber_;
    DataReader* reader_;
    Topic* topic_;
    TypeSupport type_;
    std::string participant_name_;

    class SubListener : public DataReaderListener
    {
    public:
        SubListener()
            : samples_(0)
        {
        }

        ~SubListener() override
        {
        }

        void on_subscription_matched(
                DataReader*,
                const SubscriptionMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
            {
                std::cout << "Subscriber matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                std::cout << "Subscriber unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                          << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
            }
        }

        void on_data_available(
                DataReader* reader) override
        {
            SampleInfo info;
            if (reader->take_next_sample(&hello_, &info) == eprosima::fastdds::dds::RETCODE_OK)
            {
                if (info.valid_data)
                {
                    samples_++;
                    std::cout << "Subscriber Message: " << hello_.message() << " with index: " << hello_.index()
                              << " RECEIVED." << std::endl;
                }
            }
        }

        HelloWorld hello_;
        std::atomic_int samples_;
    }
    listener_;

public:
    HelloWorldSubscriber(const std::string& participant_name = "Participant_subscriber")
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , type_(new HelloWorldPubSubType())
        , participant_name_(participant_name)
    {
    }

    virtual ~HelloWorldSubscriber()
    {
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    //!Initialize the subscriber
    bool init()
    {
        DomainParticipantQos participantQos;
        participantQos.name(participant_name_);
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

        if (participant_ == nullptr)
        {
            return false;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the subscriptions Topic
        topic_ = participant_->create_topic("HelloWorldTopic", "HelloWorld", TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            return false;
        }

        // Create the Subscriber
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

        if (subscriber_ == nullptr)
        {
            return false;
        }

        // Create the DataReader
        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);

        if (reader_ == nullptr)
        {
            return false;
        }

        return true;
    }

    //!Run the Subscriber
    void run(
            uint32_t samples,
            std::atomic_bool& stop_flag)
    {
        while (listener_.samples_ < samples && !stop_flag.load())
        {
            // 실제 시간 사용
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (listener_.samples_ >= samples)
        {
            std::cout << "Subscriber [" << participant_name_ << "] has received all samples." << std::endl;
        }
    }
};

// 시뮬레이터 클래스 정의
class HelloWorldSimulator
{
private:
    std::vector<std::shared_ptr<HelloWorldPublisher>> publishers_;
    std::vector<std::shared_ptr<HelloWorldSubscriber>> subscribers_;
    std::vector<std::thread> pub_threads_;
    std::vector<std::thread> sub_threads_;
    std::atomic_bool stop_flag_;
    uint32_t num_samples_;

public:
    HelloWorldSimulator(uint32_t num_samples = 10)
        : stop_flag_(false)
        , num_samples_(num_samples)
    {
    }

    ~HelloWorldSimulator()
    {
        // 모든 스레드 중지
        stop_flag_.store(true);
        
        // 스레드 종료 대기
        for (auto& t : pub_threads_)
        {
            if (t.joinable())
            {
                t.join();
            }
        }
        
        for (auto& t : sub_threads_)
        {
            if (t.joinable())
            {
                t.join();
            }
        }
        
        // 리소스 정리
        publishers_.clear();
        subscribers_.clear();
    }

    // Publisher 추가
    bool add_publisher(const std::string& name = "")
    {
        std::string pub_name = name.empty() ? "Publisher_" + std::to_string(publishers_.size() + 1) : name;
        auto pub = std::make_shared<HelloWorldPublisher>(pub_name);
        if (pub->init())
        {
            publishers_.push_back(pub);
            return true;
        }
        return false;
    }

    // Subscriber 추가
    bool add_subscriber(const std::string& name = "")
    {
        std::string sub_name = name.empty() ? "Subscriber_" + std::to_string(subscribers_.size() + 1) : name;
        auto sub = std::make_shared<HelloWorldSubscriber>(sub_name);
        if (sub->init())
        {
            subscribers_.push_back(sub);
            return true;
        }
        return false;
    }

    // 시뮬레이션 실행
    void run()
    {
        std::cout << "Starting simulator with " << publishers_.size() << " publishers and " 
                  << subscribers_.size() << " subscribers." << std::endl;
        
        // Publisher 스레드 시작
        for (auto& pub : publishers_)
        {
            pub_threads_.push_back(std::thread([this, pub]() {
                pub->run(num_samples_, stop_flag_);
            }));
        }
        
        // Subscriber 스레드 시작
        for (auto& sub : subscribers_)
        {
            sub_threads_.push_back(std::thread([this, sub]() {
                sub->run(num_samples_, stop_flag_);
            }));
        }
        
        // 모든 스레드 종료 대기
        for (auto& t : pub_threads_)
        {
            t.join();
        }
        
        for (auto& t : sub_threads_)
        {
            t.join();
        }
        
        std::cout << "Simulation complete." << std::endl;
    }
};

int main(int argc, char** argv)
{
    std::cout << "Starting DDS Simulator." << std::endl;
    
    // 시뮬레이션 시간 초기화 - 필요 없으므로 제거
    // 대신 현재 시간 표시
    auto current_time = std::chrono::system_clock::now();
    std::cout << "Current system time: " << std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time.time_since_epoch()).count() << " ms" << std::endl;
    
    // 수정된 FastDDS 시간 확인 (FastDDS 시간은 항상 2023년 1월 1일 0시 0분 0.123456789초로 설정됨)
    std::cout << "SimTime: 시간이 2023년 1월 1일 00:00:00.123456789으로 고정됩니다." << std::endl;
    
    // 샘플 수 설정 (기본값 10)
    uint32_t samples = 10;
    
    // 시뮬레이터 생성
    HelloWorldSimulator simulator(samples);
    
    // Publisher 추가
    if (!simulator.add_publisher("Publisher_1"))
    {
        std::cerr << "Failed to create publisher." << std::endl;
        return 1;
    }
    
    // Subscriber 추가
    if (!simulator.add_subscriber("Subscriber_1"))
    {
        std::cerr << "Failed to create subscriber." << std::endl;
        return 1;
    }
    
    // 필요에 따라 더 많은 Publisher와 Subscriber를 추가할 수 있습니다.
    // simulator.add_publisher("Publisher_2");
    // simulator.add_subscriber("Subscriber_2");
    
    // 시뮬레이션 실행
    simulator.run();
    
    return 0;
} 