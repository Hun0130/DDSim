// Copyright 2023 DDSim Project
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
 * @file DummyTransportDescriptor.hpp
 * This file provides dummy implementation for all transport descriptors
 * to be used when actual network transports are not required.
 */

#ifndef FASTDDS_RTPS_TRANSPORT__DUMMYTRANSPORTDESCRIPTOR_HPP
#define FASTDDS_RTPS_TRANSPORT__DUMMYTRANSPORTDESCRIPTOR_HPP

#include <fastdds/rtps/transport/SocketTransportDescriptor.hpp>
#include <fastdds/fastdds_dll.hpp>
#include <vector>
#include <array>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * 간단한 TCP 전송 디스크립터의 기본 클래스 정의
 * @ingroup TRANSPORT_MODULE
 */
class TCPTransportDescriptor : public SocketTransportDescriptor
{
public:
    //! 리스닝 포트 목록
    std::vector<uint16_t> listening_ports;

    //! 가상 소멸자
    virtual ~TCPTransportDescriptor() = default;

    //! 생성자
    TCPTransportDescriptor()
        : SocketTransportDescriptor(65500, 4)
    {
    }

    //! 빈 함수
    bool empty() const
    {
        return listening_ports.empty();
    }
};

/**
 * Base Dummy Transport Descriptor that serves as common parent
 * for all specific dummy transport descriptors.
 * @ingroup TRANSPORT_MODULE
 */
struct DummyTransportDescriptor : public SocketTransportDescriptor
{
    //! Destructor
    virtual ~DummyTransportDescriptor() = default;

    //! 함수 선언과 구현을 하나로 통합
    virtual TransportInterface* create_transport() const override
    {
        return nullptr;
    }

    //! Constructor
    FASTDDS_EXPORTED_API DummyTransportDescriptor();

    //! Copy constructor
    FASTDDS_EXPORTED_API DummyTransportDescriptor(
            const DummyTransportDescriptor& t) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API DummyTransportDescriptor& operator =(
            const DummyTransportDescriptor& t) = default;

    //! Compare two DummyTransportDescriptor for equality
    FASTDDS_EXPORTED_API bool operator ==(
            const DummyTransportDescriptor& t) const;
};

/**
 * UDP IPv4 Dummy Transport Descriptor
 * @ingroup TRANSPORT_MODULE
 */
struct UDPv4TransportDescriptor : public DummyTransportDescriptor
{
    //! Destructor
    virtual ~UDPv4TransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! Constructor
    FASTDDS_EXPORTED_API UDPv4TransportDescriptor();

    //! Copy constructor
    FASTDDS_EXPORTED_API UDPv4TransportDescriptor(
            const UDPv4TransportDescriptor& t) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API UDPv4TransportDescriptor& operator =(
            const UDPv4TransportDescriptor& t) = default;

    //! Compare two UDPv4TransportDescriptor for equality
    FASTDDS_EXPORTED_API bool operator ==(
            const UDPv4TransportDescriptor& t) const;
};

/**
 * UDP IPv6 Dummy Transport Descriptor
 * @ingroup TRANSPORT_MODULE
 */
struct UDPv6TransportDescriptor : public DummyTransportDescriptor
{
    //! Destructor
    virtual ~UDPv6TransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! Constructor
    FASTDDS_EXPORTED_API UDPv6TransportDescriptor();

    //! Copy constructor
    FASTDDS_EXPORTED_API UDPv6TransportDescriptor(
            const UDPv6TransportDescriptor& t) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API UDPv6TransportDescriptor& operator =(
            const UDPv6TransportDescriptor& t) = default;

    //! Compare two UDPv6TransportDescriptor for equality
    FASTDDS_EXPORTED_API bool operator ==(
            const UDPv6TransportDescriptor& t) const;
};

/**
 * TCP IPv4 Dummy Transport Descriptor
 * @ingroup TRANSPORT_MODULE
 */
struct TCPv4TransportDescriptor : public DummyTransportDescriptor
{
    //! Destructor
    virtual ~TCPv4TransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! TCP에 필요한 리스닝 포트 목록
    std::vector<uint16_t> listening_ports;

    //! WAN 주소 정보 (TCPv4에 필요)
    std::array<octet, 4> wan_addr;

    //! Constructor
    FASTDDS_EXPORTED_API TCPv4TransportDescriptor();

    //! Copy constructor
    FASTDDS_EXPORTED_API TCPv4TransportDescriptor(
            const TCPv4TransportDescriptor& t) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API TCPv4TransportDescriptor& operator =(
            const TCPv4TransportDescriptor& t) = default;

    //! Compare two TCPv4TransportDescriptor for equality
    FASTDDS_EXPORTED_API bool operator ==(
            const TCPv4TransportDescriptor& t) const;

    //! Add a port to the listening ports list
    FASTDDS_EXPORTED_API void add_listener_port(
            uint16_t port);
};

/**
 * TCP IPv6 Dummy Transport Descriptor
 * @ingroup TRANSPORT_MODULE
 */
struct TCPv6TransportDescriptor : public DummyTransportDescriptor
{
    //! Destructor
    virtual ~TCPv6TransportDescriptor() = default;

    virtual TransportInterface* create_transport() const override;

    //! TCP에 필요한 리스닝 포트 목록
    std::vector<uint16_t> listening_ports;

    //! Constructor
    FASTDDS_EXPORTED_API TCPv6TransportDescriptor();

    //! Copy constructor
    FASTDDS_EXPORTED_API TCPv6TransportDescriptor(
            const TCPv6TransportDescriptor& t) = default;

    //! Copy assignment
    FASTDDS_EXPORTED_API TCPv6TransportDescriptor& operator =(
            const TCPv6TransportDescriptor& t) = default;

    //! Compare two TCPv6TransportDescriptor for equality
    FASTDDS_EXPORTED_API bool operator ==(
            const TCPv6TransportDescriptor& t) const;

    //! Add a port to the listening ports list
    FASTDDS_EXPORTED_API void add_listener_port(
            uint16_t port);
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT__DUMMYTRANSPORTDESCRIPTOR_HPP 