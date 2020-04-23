/* -*- C -*- */
/*
 * COPYRIGHT 2012 XYRATEX TECHNOLOGY LIMITED
 *
 * THIS DRAWING/DOCUMENT, ITS SPECIFICATIONS, AND THE DATA CONTAINED
 * HEREIN, ARE THE EXCLUSIVE PROPERTY OF XYRATEX TECHNOLOGY
 * LIMITED, ISSUED IN STRICT CONFIDENCE AND SHALL NOT, WITHOUT
 * THE PRIOR WRITTEN PERMISSION OF XYRATEX TECHNOLOGY LIMITED,
 * BE REPRODUCED, COPIED, OR DISCLOSED TO A THIRD PARTY, OR
 * USED FOR ANY PURPOSE WHATSOEVER, OR STORED IN A RETRIEVAL SYSTEM
 * EXCEPT AS ALLOWED BY THE TERMS OF XYRATEX LICENSES AND AGREEMENTS.
 *
 * YOU SHOULD HAVE RECEIVED A COPY OF XYRATEX'S LICENSE ALONG WITH
 * THIS RELEASE. IF NOT PLEASE CONTACT A XYRATEX REPRESENTATIVE
 * http://www.xyratex.com/contact
 *
 * Original author: Mandar Sawant <mandar_sawant@xyratex.com>
 * Original creation date: 16/04/2012
 */

#define M0_TRACE_SUBSYSTEM M0_TRACE_SUBSYS_SNSCM
#include "lib/memory.h"
#include "lib/assert.h"
#include "lib/errno.h"
#include "lib/trace.h"
#include "lib/misc.h"

#include "reqh/reqh_service.h"
#include "sns/cm/trigger_fop.h"
#include "sns/cm/cm.h"
#include "sns/cm/service.h"
#include "sns/cm/sns_cp_onwire.h"

/**
  @defgroup SNSCMSVC SNS copy machine service
  @ingroup SNSCM

  @{
*/

/** Copy machine service type operations.*/
static int repair_svc_allocate(struct m0_reqh_service **service,
			       const struct m0_reqh_service_type *stype);

static const struct m0_reqh_service_type_ops repair_svc_type_ops = {
	.rsto_service_allocate = repair_svc_allocate,
};

M0_CM_TYPE_DECLARE(sns_repair, M0_CM_REPAIR_OPCODE,
		   &repair_svc_type_ops, "M0_CST_SNS_REP", M0_CST_SNS_REP);

/** Copy machine service operations.*/
static int repair_svc_start(struct m0_reqh_service *service);
static void repair_svc_stop(struct m0_reqh_service *service);

static const struct m0_reqh_service_ops repair_svc_ops = {
	.rso_start       = repair_svc_start,
	.rso_start_async = m0_reqh_service_async_start_simple,
	.rso_stop        = repair_svc_stop,
	.rso_fini        = m0_sns_cm_svc_fini
};

M0_EXTERN const struct m0_cm_ops sns_repair_ops;
M0_EXTERN const struct m0_fom_type_ops repair_cp_fom_type_ops;

M0_INTERNAL void m0_sns_cm_repair_cpx_init(void);
M0_INTERNAL void m0_sns_cm_repair_cpx_fini(void);

/**
 * Allocates and initialises SNS copy machine.
 * This allocates struct m0_sns_cm and invokes m0_cm_init() to initialise
 * m0_sns_cm::rc_base.
 */
static int repair_svc_allocate(struct m0_reqh_service **service,
			       const struct m0_reqh_service_type *stype)
{
	M0_ENTRY("stype: %p", stype);
	M0_PRE(service != NULL && stype != NULL);
	return M0_RC(m0_sns_cm_svc_allocate(service, stype, &repair_svc_ops,
					    &sns_repair_ops));
}

static int repair_svc_start(struct m0_reqh_service *service)
{
	int rc;

	rc = m0_sns_cm_svc_start(service);
	if (rc == 0) {
		m0_cm_cp_init(&sns_repair_cmt, &repair_cp_fom_type_ops);
		m0_sns_cm_repair_cpx_init();
		m0_sns_cm_repair_sw_onwire_fop_init();
		m0_sns_cm_repair_trigger_fop_init();
	}
	return M0_RC(rc);
}

static void repair_svc_stop(struct m0_reqh_service *service)
{
	m0_sns_cm_svc_stop(service);
	m0_sns_cm_repair_cpx_fini();
	m0_sns_cm_repair_sw_onwire_fop_fini();
	m0_sns_cm_repair_trigger_fop_fini();
}

#undef M0_TRACE_SUBSYSTEM
/** @} SNSCMSVC */

/*
 *  Local variables:
 *  c-indentation-style: "K&R"
 *  c-basic-offset: 8
 *  tab-width: 8
 *  fill-column: 80
 *  scroll-step: 1
 *  End:
 */