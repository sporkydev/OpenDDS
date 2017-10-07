/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "RtpsUdpTestLoader.h"
#include "RtpsUdpTestInst.h"

#include "dds/DCPS/transport/framework/TransportRegistry.h"
#include "dds/DCPS/transport/framework/TransportType.h"

namespace {
  const char RTPS_UDP_TEST_NAME[] = "rtps_udp";
}

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

class RtpsUdpTestType : public TransportType {
public:
  const char* name() { return RTPS_UDP_TEST_NAME; }

  TransportInst_rch new_inst(const OPENDDS_STRING& name)
  {
    return make_rch<RtpsUdpTestInst>(name);
  }
};

int
RtpsUdpTestLoader::init(int /*argc*/, ACE_TCHAR* /*argv*/[])
{
  static bool initialized(false);

  if (initialized) return 0;  // already initialized

  TransportRegistry* registry = TheTransportRegistry;
  registry->register_type(make_rch<RtpsUdpTestType>());
  // Don't create a default for RTPS.  At least for the initial implementation,
  // the user needs to explicitly configure it...
#ifdef OPENDDS_SAFETY_PROFILE
  // ...except for Safety Profile where RTPS is the only option.
  TransportInst_rch default_inst =
    registry->create_inst(TransportRegistry::DEFAULT_INST_PREFIX +
                          OPENDDS_STRING("0600_RTPS_UDP_TEST"),
                          RTPS_UDP_TEST_NAME);
  registry->get_config(TransportRegistry::DEFAULT_CONFIG_NAME)
    ->sorted_insert(default_inst);
#endif
  initialized = true;

  return 0;
}

ACE_FACTORY_DEFINE(OpenDDS_Rtps_Udp_Test, RtpsUdpTestLoader);
ACE_STATIC_SVC_DEFINE(
  RtpsUdpTestLoader,
  ACE_TEXT("OpenDDS_Rtps_Udp_Test"),
  ACE_SVC_OBJ_T,
  &ACE_SVC_NAME(RtpsUdpTestLoader),
  ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ,
  0)

} // namespace DCPS
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL
