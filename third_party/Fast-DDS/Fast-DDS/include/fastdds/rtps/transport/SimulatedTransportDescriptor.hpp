// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SimulatedTransportDescriptor.hpp
 */

#ifndef _FASTDDS_RTPS_TRANSPORT_SIMULATEDTRANSPORTDESCRIPTOR_HPP_
#define _FASTDDS_RTPS_TRANSPORT_SIMULATEDTRANSPORTDESCRIPTOR_HPP_

#include <fastdds/rtps/transport/TransportDescriptorInterface.hpp>
#include <string>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * 네트워크 레이어 시뮬레이션 모드 열거형
 */
enum class NetworkSimulationMode {
    SIMPLE,     // 간단한 지연, 손실만 시뮬레이션
    REALISTIC,  // 실제 네트워크와 유사한 복합적 시뮬레이션
    CUSTOM      // 사용자 정의 시뮬레이션
};

/**
 * Simulated Transport configuration.
 *
 * This transport simulates network behavior without actual networking.
 * It is designed for testing and simulation purposes in a single process
 * with multiple DomainParticipants.
 *
 * @ingroup TRANSPORT_MODULE
 */
class SimulatedTransportDescriptor : public TransportDescriptorInterface
{
public:

    /**
     * Constructor with default parameter values.
     */
    FASTDDS_EXPORTED_API SimulatedTransportDescriptor();

    /**
     * Copy constructor.
     */
    FASTDDS_EXPORTED_API SimulatedTransportDescriptor(
            const SimulatedTransportDescriptor& descriptor);

    /**
     * Destructor.
     */
    virtual ~SimulatedTransportDescriptor() noexcept = default;

    /**
     * Factory method to create a SimulatedTransport.
     * This method must be implemented to create the transport from this descriptor.
     */
    FASTDDS_EXPORTED_API TransportInterface* create_transport() const override;

    /**
     * Assignment operator.
     */
    FASTDDS_EXPORTED_API SimulatedTransportDescriptor& operator =(
            const SimulatedTransportDescriptor& descriptor);

    /**
     * Creates a new copy of this descriptor.
     */
    virtual FASTDDS_EXPORTED_API TransportDescriptorInterface* clone() const;

    /**
     * Compare two descriptors.
     */
    virtual FASTDDS_EXPORTED_API bool operator ==(
            const TransportDescriptorInterface& descriptor) const;

    /**
     * Returns the minimum size required for a send operation.
     */
    virtual FASTDDS_EXPORTED_API uint32_t min_send_buffer_size() const override;

    // SimulatedTransport specific parameters

    /**
     * Maximum message size allowed in this transport
     */
    uint32_t max_message_size = 65536;

    /**
     * Maximum initial peers range
     */
    uint32_t max_initial_peers_range = 4;

    //-----------------------------------------------------------------------
    // 네트워크 레이어 시뮬레이션 설정
    //-----------------------------------------------------------------------
    /**
     * 네트워크 시뮬레이션 모드
     */
    NetworkSimulationMode network_simulation_mode = NetworkSimulationMode::SIMPLE;

    /**
     * 사용자 정의 네트워크 시뮬레이션 클래스 이름
     * network_simulation_mode가 CUSTOM일 경우 사용됨
     */
    std::string custom_network_simulation_class;

    /**
     * 호스트 ID (시뮬레이션에서 각 호스트 구분용)
     */
    uint32_t host_id = 0;

    //-----------------------------------------------------------------------
    // 패킷 손실 설정
    //-----------------------------------------------------------------------
    /**
     * Simulated packet loss rate (0.0 to 1.0)
     * 0.0 means no packet loss, 1.0 means all packets are lost
     */
    float packet_loss_rate = 0.0f;

    /**
     * 패킷 손실 패턴 (랜덤, 버스트 등)
     * 0: 랜덤 손실
     * 1: 버스트 손실 (연속된 패킷 손실)
     * 2: 주기적 손실 (특정 패턴으로 반복)
     */
    uint32_t packet_loss_pattern = 0;

    /**
     * 버스트 손실 시 평균 버스트 길이
     */
    uint32_t packet_loss_burst_length = 1;

    //-----------------------------------------------------------------------
    // 패킷 손상 설정
    //-----------------------------------------------------------------------
    /**
     * Simulated packet corruption rate (0.0 to 1.0)
     * 0.0 means no corruption, 1.0 means all packets are corrupted
     */
    float packet_corruption_rate = 0.0f;

    /**
     * 손상 패턴 (비트 플립, 바이트 손상 등)
     * 0: 랜덤 비트 플립
     * 1: 바이트 손상
     * 2: 헤더 손상
     */
    uint32_t corruption_pattern = 0;

    /**
     * 손상시 영향을 받는 데이터 비율 (0.0 ~ 1.0)
     */
    float corruption_data_ratio = 0.1f;

    //-----------------------------------------------------------------------
    // 지연 설정
    //-----------------------------------------------------------------------
    /**
     * Simulated network delay in milliseconds
     */
    uint32_t network_delay_ms = 0;

    /**
     * Jitter in milliseconds (random delay variance)
     */
    uint32_t delay_jitter_ms = 0;

    /**
     * 지연 패턴 (고정, 정규분포, 지터 등)
     * 0: 고정 지연
     * 1: 정규분포 지연
     * 2: 주기적 변동 지연
     */
    uint32_t delay_pattern = 0;

    //-----------------------------------------------------------------------
    // 대역폭 및 혼잡 설정
    //-----------------------------------------------------------------------
    /**
     * Network bandwidth limit in bytes per second (0 = unlimited)
     */
    uint32_t bandwidth_limit_bps = 0;

    /**
     * Enable simulated network congestion
     */
    bool enable_congestion = false;

    /**
     * Congestion window size in bytes
     */
    uint32_t congestion_window_size = 65536;

    /**
     * 혼잡 패턴 (점진적, 급격한 변화 등)
     * 0: 점진적 혼잡 변화
     * 1: 급격한 혼잡 변화
     * 2: 주기적 혼잡 변화
     */
    uint32_t congestion_pattern = 0;

    /**
     * 혼잡 회복 속도 계수 (0.1 ~ 10.0)
     */
    float congestion_recovery_factor = 1.0f;

    //-----------------------------------------------------------------------
    // 기타 시뮬레이션 설정
    //-----------------------------------------------------------------------
    /**
     * Simulate discovery delay in milliseconds (0 = no delay)
     */
    uint32_t discovery_delay_ms = 0;

    /**
     * Unique transport ID to identify this simulated transport instance
     */
    uint32_t transport_id = 0;

    /**
     * Auto-increment participant IDs (used for simulation identification)
     */
    static int next_participant_id;

    /**
     * Static flag to enable time-based simulation
     * When true, the transport uses a global simulation clock that can
     * be controlled externally
     */
    static bool enable_time_simulation;

    /**
     * Static factor to scale simulation time relative to real time
     * Values > 1.0 make simulation run slower than real time
     * Values < 1.0 make simulation run faster than real time
     */
    static float time_scale_factor;

    /**
     * 패킷 캡처 사용 여부 (디버깅 및 분석용)
     */
    bool enable_packet_capture = false;

    /**
     * 패킷 캡처 파일 경로
     */
    std::string packet_capture_file = "simulated_transport_capture.pcap";
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_RTPS_TRANSPORT_SIMULATEDTRANSPORTDESCRIPTOR_HPP_ 