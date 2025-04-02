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
 * @file SimulatedTransportDescriptor.cpp
 */

#include <fastdds/rtps/transport/SimulatedTransportDescriptor.hpp>
#include <fastdds/rtps/transport/SimulatedTransport.hpp>

#include <utility>

namespace eprosima {
namespace fastdds {
namespace rtps {

// 정적 멤버 변수 초기화
int SimulatedTransportDescriptor::next_participant_id = 0;
bool SimulatedTransportDescriptor::enable_time_simulation = false;
float SimulatedTransportDescriptor::time_scale_factor = 1.0f;

SimulatedTransportDescriptor::SimulatedTransportDescriptor()
    : TransportDescriptorInterface(max_message_size, max_initial_peers_range)
{
}

SimulatedTransportDescriptor::SimulatedTransportDescriptor(
        const SimulatedTransportDescriptor& descriptor)
    : TransportDescriptorInterface(descriptor)
{
    packet_loss_rate = descriptor.packet_loss_rate;
    packet_corruption_rate = descriptor.packet_corruption_rate;
    network_delay_ms = descriptor.network_delay_ms;
    delay_jitter_ms = descriptor.delay_jitter_ms;
    bandwidth_limit_bps = descriptor.bandwidth_limit_bps;
    enable_congestion = descriptor.enable_congestion;
    congestion_window_size = descriptor.congestion_window_size;
    discovery_delay_ms = descriptor.discovery_delay_ms;
    transport_id = descriptor.transport_id;
    max_message_size = descriptor.max_message_size;
    max_initial_peers_range = descriptor.max_initial_peers_range;
}

TransportInterface* SimulatedTransportDescriptor::create_transport() const
{
    // 임시로 nullptr 반환 - Fast-DDS 빌드만 통과하기 위함
    // DDSim 프로젝트에서는 별도로 SimulatedTransportAdapter 인스턴스 생성
    std::cout << "SimulatedTransportDescriptor::create_transport: 아직 구현되지 않음" << std::endl;
    return nullptr;
}

SimulatedTransportDescriptor& SimulatedTransportDescriptor::operator =(
        const SimulatedTransportDescriptor& descriptor)
{
    TransportDescriptorInterface::operator=(descriptor);
    packet_loss_rate = descriptor.packet_loss_rate;
    packet_corruption_rate = descriptor.packet_corruption_rate;
    network_delay_ms = descriptor.network_delay_ms;
    delay_jitter_ms = descriptor.delay_jitter_ms;
    bandwidth_limit_bps = descriptor.bandwidth_limit_bps;
    enable_congestion = descriptor.enable_congestion;
    congestion_window_size = descriptor.congestion_window_size;
    discovery_delay_ms = descriptor.discovery_delay_ms;
    transport_id = descriptor.transport_id;
    max_message_size = descriptor.max_message_size;
    max_initial_peers_range = descriptor.max_initial_peers_range;
    return *this;
}

TransportDescriptorInterface* SimulatedTransportDescriptor::clone() const
{
    return new SimulatedTransportDescriptor(*this);
}

bool SimulatedTransportDescriptor::operator ==(
        const TransportDescriptorInterface& descriptor) const
{
    const auto simulated_descriptor = dynamic_cast<const SimulatedTransportDescriptor*>(&descriptor);
    if (simulated_descriptor == nullptr)
    {
        return false;
    }

    return TransportDescriptorInterface::operator==(descriptor) &&
           packet_loss_rate == simulated_descriptor->packet_loss_rate &&
           packet_corruption_rate == simulated_descriptor->packet_corruption_rate &&
           network_delay_ms == simulated_descriptor->network_delay_ms &&
           delay_jitter_ms == simulated_descriptor->delay_jitter_ms &&
           bandwidth_limit_bps == simulated_descriptor->bandwidth_limit_bps &&
           enable_congestion == simulated_descriptor->enable_congestion &&
           congestion_window_size == simulated_descriptor->congestion_window_size &&
           discovery_delay_ms == simulated_descriptor->discovery_delay_ms &&
           transport_id == simulated_descriptor->transport_id;
}

uint32_t SimulatedTransportDescriptor::min_send_buffer_size() const
{
    return 0;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima 