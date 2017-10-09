/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#ifndef DCPS_RTPS_UDP_TEST_H
#define DCPS_RTPS_UDP_TEST_H

#include "Rtps_Udp_Test_Export.h"
#include "dds/Versioned_Namespace.h"

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

class OpenDDS_Rtps_Udp_Test_Export RtpsUdpTestInitializer {
public:
  RtpsUdpTestInitializer();
};

static RtpsUdpTestInitializer rtps_udp_test_init;

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL

#endif  /* DCPS_UDP_H */
