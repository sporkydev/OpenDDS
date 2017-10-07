/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "RtpsUdpTest.h"
#include "RtpsUdpTestLoader.h"

#include "ace/Service_Config.h"

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

RtpsUdpTestInitializer::RtpsUdpTestInitializer()
{
  ACE_Service_Config::process_directive(ace_svc_desc_RtpsUdpTestLoader);

#if (OPENDDS_RTPS_UDP_TEST_HAS_DLL == 0)
  ACE_Service_Config::process_directive(ACE_TEXT("static OpenDDS_Rtps_Udp_Test"));
#endif  /* OPENDDS_RTPS_UDP_TEST_HAS_DLL */
}

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL
