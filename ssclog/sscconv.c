/*
    SKYLINE SOARING CLUB DUTY LOGSHEET PROGRAM - Converter section

    Copyright (c) 2004-2011 Jonathan A. Kans. All rights reserved.
    current maintainer: https://github.com/ertantete/ssclog
*/

#include "sscconv.h"

CharPtr data_spec_version = "8.4.2021";

#define OLD_REG_MEMB_CHARGE           1
#define OLD_SSC_DUES_CHARGE           2
#define OLD_SSA_DUES_CHARGE           3
#define OLD_INTRO_MEMB_CHARGE         4
#define OLD_VISITING_MEMB_CHARGE      5
#define OLD_AFFIL_MEMB_CHARGE         6
#define OLD_TRANS_MEMB_CHARGE         7
#define OLD_AERIAL_RETRIEVE_CHARGE    8
#define OLD_BOOK_SALE_CHARGE          9
#define OLD_VIDEO_SALE_CHARGE        10
#define OLD_CLOTHING_SALE_CHARGE     11
#define OLD_PACKET_SALE_CHARGE       12
#define OLD_MERCHANDISE_SALE_CHARGE  13
#define OLD_TEMP_MEMB_CHARGE         14  /* obsolete, maps to OLD_INTRO_MEMB_CHARGE */
#define OLD_SPEC_MEMB_CHARGE         15  /* obsolete, maps to OLD_TRANS_MEMB_CHARGE */
#define OLD_OTHER_CHARGE             16

#define OLD_FULL_MEMBER       1
#define OLD_FAST_MEMBER       2
#define OLD_INTRO_MEMBER      3
#define OLD_VISITING_MEMBER   4
#define OLD_AFFIL_MEMBER      5
#define OLD_TRANSIENT_MEMBER  6
#define OLD_SPECIAL_MEMBER    7  /* obsolete, maps to OLD_TRANSIENT_MEMBER */
#define OLD_SERVICE_MEMBER    8
#define OLD_GUEST_MEMBER      9
#define OLD_PAYER_MEMBER     10
#define OLD_OTHER_MEMBER     11

/* copied from ssclog.h */

#define REG_MEMB_FEE         1
#define SSC_DUES_FEE         2
#define SSA_DUES_FEE         3
#define INTRO_MEMB_FEE       4
#define VISITING_MEMB_FEE    5
#define AFFIL_MEMB_FEE       6
#define TRANS_MEMB_FEE       7
#define AERIAL_RETRIEVE_FEE  8
#define OTHER_FEE            9

#define VOUCHER_SALE_CHARGE      1
#define BOOK_SALE_CHARGE         2
#define VIDEO_SALE_CHARGE        3
#define CLOTHING_SALE_CHARGE     4
#define PACKET_SALE_CHARGE       5
#define MERCHANDISE_SALE_CHARGE  6
#define OTHER_CHARGE             7

extern void ModernizeLogSheet (
  LogSheetPtr lsp
)

{
  ContactInfoPtr   cip;
  ChargeDataPtr    cdp, lastcdp = NULL;
  Int4             chargeReason;
  ClockTimePtr     ctp;
  FeeDataPtr       fdp, lastfdp = NULL;
  Int4             feeReason;
  GliderFlightPtr  fip;
  FlightTimePtr    ftp;
  Int4             ld_hour, ld_min, to_hour, to_min;
  MiscChargePtr    mcp, firstmcp, nextmcp;

  if (lsp == NULL) return;

  if (StringCmp (lsp->version, data_spec_version) < 0) {
    if (lsp->original_version == NULL) {
      lsp->original_version = lsp->version;
      lsp->version = NULL;
    } else {
      lsp->version = MemFree (lsp->version);
    }
    lsp->version = StringSaveNoNull (data_spec_version);
  }

  if (lsp->flights != NULL && lsp->glider_flights == NULL) {
    lsp->glider_flights = lsp->flights;
    lsp->flights = NULL;
  }

  for (fip = lsp->glider_flights; fip != NULL; fip = fip->next) {
    if (fip->landed_out && fip->exception == 0) {
      fip->exception = 1;
      fip->landed_out = FALSE;
    }
  }

  for (fip = lsp->glider_flights; fip != NULL; fip = fip->next) {
    if (fip->takeoff == NULL || fip->landing == NULL) continue;

    ftp = fip->flight_time;
    if (ftp == NULL) {
      ftp = FlightTimeNew ();
      if (ftp != NULL) {
        ftp->hours = -1;
        ftp->minutes = -1;
      }
      fip->flight_time = ftp;
    }
    if (ftp == NULL) continue;
    if (ftp->hours > -1 && ftp->minutes > -1) continue;

    ctp = fip->takeoff;
    to_hour = ctp->hour;
    to_min = ctp->minute;

    ctp = fip->landing;
    ld_hour = ctp->hour;
    ld_min = ctp->minute;

    if (to_hour == -1 || to_min == -1 || ld_hour == -1 || ld_min == -1) {
      ftp->hours = -1;
      ftp->minutes = -1;
    } else if (to_hour > ld_hour) {
      ftp->hours = -1;
      ftp->minutes = -1;
    } else if (to_min > ld_min && to_hour == ld_hour) {
      ftp->hours = -1;
      ftp->minutes = -1;
    } else if (to_min > ld_min) {
      ftp->hours = ld_hour - to_hour - 1;
      ftp->minutes = 60 - to_min + ld_min;
    } else if (to_min == ld_min && to_hour == ld_hour) {
      ftp->hours = 0;
      ftp->minutes = 0;
    } else {
      ftp->hours = ld_hour - to_hour;
      ftp->minutes = ld_min - to_min;
    }
  }

  firstmcp = lsp->other_charges;
  lsp->other_charges = NULL;

  for (mcp = firstmcp; mcp != NULL; mcp = nextmcp) {
    nextmcp = mcp->next;
    /*
    if (mcp->reason == OLD_TEMP_MEMB_CHARGE) {
      mcp->reason = OLD_INTRO_MEMB_CHARGE;
    } else if (mcp->reason == OLD_SPEC_MEMB_CHARGE) {
      mcp->reason = OLD_TRANS_MEMB_CHARGE;
    }
    */
    chargeReason = 0;
    feeReason = 0;
    switch (mcp->reason) {
      case OLD_REG_MEMB_CHARGE :
        feeReason = REG_MEMB_FEE;
        break;
      case OLD_SSC_DUES_CHARGE :
        feeReason = SSC_DUES_FEE;
        break;
      case OLD_SSA_DUES_CHARGE :
        feeReason = SSA_DUES_FEE;
        break;
      case OLD_INTRO_MEMB_CHARGE :
        feeReason = INTRO_MEMB_FEE;
        break;
      case OLD_VISITING_MEMB_CHARGE :
        feeReason = VISITING_MEMB_FEE;
        break;
      case OLD_AFFIL_MEMB_CHARGE :
        feeReason = AFFIL_MEMB_FEE;
        break;
      case OLD_TRANS_MEMB_CHARGE :
        feeReason = TRANS_MEMB_FEE;
        break;
      case OLD_AERIAL_RETRIEVE_CHARGE :
        feeReason = AERIAL_RETRIEVE_FEE;
        break;
      case OLD_BOOK_SALE_CHARGE :
        chargeReason = BOOK_SALE_CHARGE;
        break;
      case OLD_VIDEO_SALE_CHARGE :
        chargeReason = VIDEO_SALE_CHARGE;
        break;
      case OLD_CLOTHING_SALE_CHARGE :
        chargeReason = CLOTHING_SALE_CHARGE;
        break;
      case OLD_PACKET_SALE_CHARGE :
        chargeReason = PACKET_SALE_CHARGE;
        break;
      case OLD_MERCHANDISE_SALE_CHARGE :
        chargeReason = MERCHANDISE_SALE_CHARGE;
        break;
      case OLD_TEMP_MEMB_CHARGE :
        feeReason = INTRO_MEMB_FEE;
        break;
      case OLD_SPEC_MEMB_CHARGE :
        feeReason = TRANS_MEMB_FEE;
        break;
      case OLD_OTHER_CHARGE :
        chargeReason = OTHER_CHARGE;
        break;
      default :
        break;
    }

    if (feeReason != 0) {
      fdp = (FeeDataPtr) MemNew (sizeof (FeeData));
      if (fdp != NULL) {
        fdp->name = mcp->name;
        fdp->type = mcp->type;
        fdp->alt_payer = mcp->alt_payer;
        fdp->amount = mcp->amount;
        fdp->reason = feeReason;
        fdp->item = mcp->item;
        fdp->comment = mcp->comment;
        if (lastfdp != NULL) {
          lastfdp->next = fdp;
        } else {
          lsp->fees = fdp;
        }
        lastfdp = fdp;
        MemFree (mcp);
      }

    } else if (chargeReason != 0) {
      cdp = (ChargeDataPtr) MemNew (sizeof (ChargeData));
      if (cdp != NULL) {
        cdp->name = mcp->name;
        cdp->type = mcp->type;
        cdp->alt_payer = mcp->alt_payer;
        cdp->amount = mcp->amount;
        cdp->reason = chargeReason;
        cdp->item = mcp->item;
        cdp->comment = mcp->comment;
        if (lastcdp != NULL) {
          lastcdp->next = cdp;
        } else {
          lsp->charges = cdp;
        }
        lastcdp = cdp;
        MemFree (mcp);
      }
    }
  }

  for (cip = lsp->new_contacts; cip != NULL; cip = cip->next) {
    if (cip->type == OLD_SPECIAL_MEMBER) {
      cip->type = OLD_TRANSIENT_MEMBER;
    }
  }
}

extern void ExpandLogSheet (
  LogSheetPtr lsp
)

{
}

extern void ContractLogSheet (
  LogSheetPtr lsp
)

{
}

