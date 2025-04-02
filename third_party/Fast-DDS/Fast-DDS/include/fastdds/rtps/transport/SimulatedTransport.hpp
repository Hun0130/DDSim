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
 * @file SimulatedTransport.hpp
 */

#ifndef _FASTDDS_RTPS_TRANSPORT_SIMULATEDTRANSPORT_HPP_
#define _FASTDDS_RTPS_TRANSPORT_SIMULATEDTRANSPORT_HPP_

#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <random>
#include <condition_variable>
#include <chrono>
#include <memory>

#include <fastdds/rtps/transport/TransportInterface.hpp>
#include <fastdds/rtps/transport/SimulatedTransportDescriptor.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

// Forward declarations
class SimulatedChannelResource;
class SimulatedMessageQueue;

/**
 * This is a simulated transport class.
 * - It simulates network behavior without actual network communication.
 * - It provides message passing between participants within a single process.
 * - It can simulate network conditions like delay, packet loss, etc.
 * 
 * @ingroup TRANSPORT_MODULE
 */
class SimulatedTransport : public TransportInterface
{
public:
    /**
     * Constructor
     */
    FASTDDS_EXPORTED_API SimulatedTransport(
            const SimulatedTransportDescriptor& descriptor);

    /**
     * Destructor
     */
    virtual ~SimulatedTransport() override;

    // TransportInterface Implementation
    bool init(
            const PropertyPolicy* properties = nullptr,
            const uint32_t& max_msg_size_no_frag = 0) override;

    bool IsInputChannelOpen(
            const Locator& locator) const override;

    bool IsLocatorSupported(
            const Locator& locator) const override;

    bool is_locator_allowed(
            const Locator& locator) const override;

    bool OpenOutputChannel(
            SendResourceList& send_resource_list,
            const Locator& locator) override;

    bool OpenInputChannel(
            const Locator& locator,
            TransportReceiverInterface* receiver,
            uint32_t max_message_size) override;

    bool CloseInputChannel(
            const Locator& locator) override;

    bool DoInputLocatorsMatch(
            const Locator& left,
            const Locator& right) const override;

    void select_locators(
            LocatorSelector& selector) const override;

    bool is_local_locator(
            const Locator& locator) const override;

    TransportDescriptorInterface* get_configuration() override;

    void AddDefaultOutputLocator(
            LocatorList& defaultList) override;

    LocatorList NormalizeLocator(
            const Locator& locator) override;

    bool transform_remote_locator(
            const Locator& remote_locator,
            Locator& result_locator) const override;

    Locator RemoteToMainLocal(
            const Locator& remote) const override;

    bool getDefaultMetatrafficMulticastLocators(
            LocatorList& locators,
            uint32_t metatraffic_multicast_port) const override;

    bool getDefaultMetatrafficUnicastLocators(
            LocatorList& locators,
            uint32_t metatraffic_unicast_port) const override;

    bool getDefaultUnicastLocators(
            LocatorList& locators,
            uint32_t unicast_port) const override;

    bool fillMetatrafficMulticastLocator(
            Locator& locator,
            uint32_t metatraffic_multicast_port) const override;

    bool fillMetatrafficUnicastLocator(
            Locator& locator,
            uint32_t metatraffic_unicast_port) const override;

    bool configureInitialPeerLocator(
            Locator& locator,
            const PortParameters& port_params,
            uint32_t domainId,
            LocatorList& list) const override;

    bool fillUnicastLocator(
            Locator& locator,
            uint32_t well_known_port) const override;

    void shutdown() override;

    void update_network_interfaces() override;

    bool is_locator_reachable(
            const Locator& locator) override;

private:
    // Configuration
    SimulatedTransportDescriptor configuration_;
    
    // Random number generator for network simulation
    std::mt19937 random_engine_;
    
    // Thread for message processing
    std::thread message_thread_;
    bool running_;
    mutable std::mutex thread_mutex_;
    std::condition_variable thread_cv_;
    
    // Channel resources
    mutable std::mutex channels_mutex_;
    std::map<Locator, std::shared_ptr<SimulatedChannelResource>> input_channels_;
    
    // Message queues
    std::mutex queues_mutex_;
    std::map<uint32_t, std::shared_ptr<SimulatedMessageQueue>> message_queues_;
    
    // Message handling thread function
    void process_messages();
    
    // Helper methods for network simulation
    bool should_drop_message() const;
    uint32_t calculate_delay(uint32_t size) const;
    bool should_corrupt_message() const;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_RTPS_TRANSPORT_SIMULATEDTRANSPORT_HPP_ 