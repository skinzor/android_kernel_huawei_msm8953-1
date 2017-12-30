#include "lcdkit_dbg.h"

#define ESD_FAIL     0
#define ESD_NOMAL    1

//mdss_dsi_bta_status_check
int mdss_dsi_bta_status_check(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int ret = 0;

	if (ctrl_pdata == NULL)
    {
		LCDKIT_ERR("Invalid input data\n");
		/*
		 * This should not return error otherwise
		 * BTA status thread will treat it as dead panel scenario
		 * and request for blank/unblank
		 */
		return 0;
	}

	LCDKIT_DEBUG("Checking BTA status\n");

	/*if panel check error and enable the esd check bit in dtsi,report the event to hal layer*/
	if(lcdkit_info.panel_infos.esd_support)
	{
        ret = lcdkit_info.lcdkit_check_esd(ctrl_pdata);

        // in qcom code: 0 - fail; 1 - ok, but lcdkit code: 1 - fail; 0 - ok
        // we need adapt here
        ret = (ret > 0) ? ESD_FAIL : ESD_NOMAL;
	}

	return ret;
}

