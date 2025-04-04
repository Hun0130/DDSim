// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <rtps/transport/UDPChannelResource.h>

#include <asio.hpp>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>

#include <rtps/messages/MessageReceiver.h>
#include <rtps/transport/UDPTransportInterface.h>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Log = fastdds::dds::Log;

UDPChannelResource::UDPChannelResource(
        UDPTransportInterface* transport,
        eProsimaUDPSocket& socket,
        uint32_t maxMsgSize,
        const Locator& locator,
        const std::string& sInterface,
        TransportReceiverInterface* receiver,
        const ThreadSettings& thread_config)
    : ChannelResource(maxMsgSize)
    , message_receiver_(receiver)
    , socket_(moveSocket(socket))
    , only_multicast_purpose_(false)
    , interface_(sInterface)
    , transport_(transport)
{
    auto fn = [this, locator]()
            {
                perform_listen_operation(locator);
            };
    thread(create_thread(fn, thread_config, "dds.udp.%u", locator.port));
}

UDPChannelResource::~UDPChannelResource()
{
    message_receiver_ = nullptr;

    asio::error_code ec;
    socket()->close(ec);
}

void UDPChannelResource::perform_listen_operation(
        Locator input_locator)
{
    Locator remote_locator;

    while (alive())
    {
        // Blocking receive.
        auto& msg = message_buffer();
        if (!Receive(msg.buffer, msg.max_size, msg.length, remote_locator))
        {
            continue;
        }

        // Processes the data through the CDR Message interface.
        if (message_receiver() != nullptr)
        {
            message_receiver()->OnDataReceived(msg.buffer, msg.length, input_locator, remote_locator);
        }
        else if (alive())
        {
            EPROSIMA_LOG_WARNING(RTPS_MSG_IN, "Received Message, but no receiver attached");
        }
    }

    message_receiver(nullptr);
}

bool UDPChannelResource::Receive(
        octet* receive_buffer,
        uint32_t receive_buffer_capacity,
        uint32_t& receive_buffer_size,
        Locator& remote_locator)
{
    try
    {
        // SerializedOutputData가 있는지 확인하고 있다면 해당 데이터를 사용
        if (!last_serialized_data.data.empty())
        {
            EPROSIMA_LOG_INFO(RTPS_MSG_IN, "Using last serialized data instead of socket receive");
            
            // 버퍼 크기 제한 확인
            size_t bytes_to_copy = std::min(static_cast<size_t>(receive_buffer_capacity), last_serialized_data.data.size());
            receive_buffer_size = static_cast<uint32_t>(bytes_to_copy);
            
            // 데이터 복사
            memcpy(receive_buffer, last_serialized_data.data.data(), bytes_to_copy);
            
            // destination 정보를 파싱하여 원격 로케이터 설정
            // 예: "127.0.0.1:7412"에서 IP와 포트 추출
            std::string destination = last_serialized_data.destination;
            size_t pos = destination.find(":");
            
            if (pos != std::string::npos)
            {
                std::string ip = destination.substr(0, pos);
                int port = std::stoi(destination.substr(pos + 1));
                
                asio::ip::udp::endpoint endpoint(asio::ip::address::from_string(ip), port);
                transport_->endpoint_to_locator(endpoint, remote_locator);
            }
            else
            {
                // 기본값 설정
                asio::ip::udp::endpoint endpoint(asio::ip::address::from_string("127.0.0.1"), 7412);
                transport_->endpoint_to_locator(endpoint, remote_locator);
            }
            
            // 데이터 처리 후 비우기
            last_serialized_data.data.clear();
            
            EPROSIMA_LOG_INFO(RTPS_MSG_IN, "Processed serialized data, size: " << receive_buffer_size);
            return (receive_buffer_size > 0);
        }
        
        return false;
    }
    catch (const std::exception& error)
    {
        (void)error;
        EPROSIMA_LOG_WARNING(RTPS_MSG_OUT, "Error processing data: " << error.what() << " - " << message_receiver()
                                                                    << " (" << this << ")");
        return false;
    }
}

void UDPChannelResource::release()
{
    // Cancel all asynchronous operations associated with the socket.
    socket()->cancel();
    // Disable receives on the socket.
    // shutdown always returns a 'shutdown: Transport endpoint is not connected' error,
    // since the endpoint is indeed not connected. However, it unblocks the synchronous receive
    // in Windows and Linux anyways, which is what we want.
    asio::error_code ec;
    socket()->shutdown(asio::socket_base::shutdown_type::shutdown_receive, ec);

#if defined(__APPLE__)
    // On OSX shutdown does not seem to unblock the listening thread, but close does.
    socket()->close();
#endif // if defined(__APPLE__)
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
