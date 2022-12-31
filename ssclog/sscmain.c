/*
    SKYLINE SOARING CLUB DUTY LOGSHEET PROGRAM - Main section

    Copyright (c) 2004-2011 Jonathan A. Kans. All rights reserved.

    Ver   Date       Author           Comment
    ----- ---------- ---------------- ------------------------------------------
    1.0              Jonathan Kans    Created
    1.1   01/03/2016 Ertan Tete       Added years 2016-2019 to dst_start and
                                      dst_stop
                                      Extended the validation procedure to check
                                      if airport is specified.
                                      Modified the validation procedure to
                                      issue waring in case the selected airport 
                                      is different from the home airport (home
                                      airport is the first airport in file
                                      airports.txt)
                                      Changed program_version to 8E
   1.2   12/20/2019 Chris Norris      Added 2020, program_version++
   1.3   01/01/2021 Chris Norris      Added 2021, 2022
   1.4   01/02/2021 Ertan Tete        program_version++
   1.5   12/21/2022 Ertan Tete        Added years 2023-2025, program_version++.
                                      Enhanced start_tach, end_tach, and 
				      gas_added validation.
				      Validate tach time and gas added data of
				      both towplanes (if both were used).

*/

/*
    asntool -m ssc.asn -o asnssc.h; asntool -m ssc.asn -G -Z -B objssc -K asnssc.h
*/

#include "ssclog.h"

/* change data_spec_version in sscconv.c */

CharPtr program_version = "8H";

typedef struct subpage {
  GrouP  content;
  GrouP  pages [10];
  Int2   currPage;
} SubPage, PNTR SubPagePtr;


typedef struct sscform {
  FORM_MESSAGE_BLOCK

  SubPage    mainPage;

  SubPage    startupPage;
  SubPage    personnelPage;
  SubPage    operationsPage;
  SubPage    financesPage;
  SubPage    shutdownPage;

  MenU       editMenu;
  MenU       flightMenu;
  MenU       paymentMenu;
  MenU       personMenu;

  DoC        helpDoc;

  SscTables  tables;
} SscForm, PNTR SscFormPtr;

#ifdef WIN_MAC
#define SYS_FOLDER_TAB systemFont,20,5,2,6,10
#define PROG_FOLDER_TAB programFont,10,5,2,4,5
#else
#define SYS_FOLDER_TAB programFont,20,5,2,6,10
#define PROG_FOLDER_TAB systemFont,10,5,2,4,5
#endif


#ifdef WIN_MAC
static IteM  cutItem = NULL;
static IteM  copyItem = NULL;
static IteM  pasteItem = NULL;
static IteM  deleteItem = NULL;
#endif

static CharPtr startupTitles [] = {
  "Staff & Settings",
  "Potomac TRACON",
  "Program Help",
  NULL
};

static CharPtr altStartupTitles [] = {
  "Staff & Settings",
  "Program Help",
  NULL
};

static void ChangeStartupTabs (
  VoidPtr data,
  Int2 newval,
  Int2 oldval
)

{
  SscFormPtr  sfp;
  SubPagePtr  spp;

  sfp = (SscFormPtr) data;
  if (sfp == NULL) return;

  spp = &(sfp->startupPage);

  SafeHide (spp->pages [oldval]);
  Update ();

  spp->currPage = newval;
  SafeShow (spp->pages [newval]);
  Update ();
}

static void ChangeDate (
  DialoG d
)

{
  BaseDialogPtr  bdp;
  SscFormPtr     sfp;

  bdp = (BaseDialogPtr) GetObjectExtra (d);
  if (bdp == NULL) return;
  sfp = (SscFormPtr) bdp->userdata;
  if (sfp == NULL) return;

  SetDirtyFlag (&(sfp->tables));
}

#ifdef WIN_MSWIN
static ENUM_ALIST(flash_alist)
  {" ",  0},
  {"D",  1},
  {"E",  2},
  {"F",  3},
  {"G",  4},
  {"H",  5},
END_ENUM_ALIST
#endif

static void CreateStartupGroup (
  GrouP p,
  SscFormPtr sfp,
  Int2 wid
)

{
  GrouP         g, h, x, y = NULL;
  Int2          page = 0;
  SubPagePtr    spp;
  SscTablesPtr  stp;
  DialoG        tbs;
  Boolean       usesTracon;

  if (sfp == NULL) return;
  spp = &(sfp->startupPage);
  if (spp == NULL) return;
  stp = &(sfp->tables);
  if (stp == NULL) return;

  usesTracon = (Boolean) (StringICmp (stp->traconname, "Potomac TRACON") == 0);
  if (usesTracon) {
    tbs = CreateFolderTabs (p, startupTitles, 0, 0, 0, PROG_FOLDER_TAB,
                            ChangeStartupTabs, (Pointer) sfp);
  } else {
    tbs = CreateFolderTabs (p, altStartupTitles, 0, 0, 0, PROG_FOLDER_TAB,
                            ChangeStartupTabs, (Pointer) sfp);
  }

  h = HiddenGroup (p, 0, 0, NULL);
  spp->content = h;
  SetGroupMargins (h, 10, 10);

  g = HiddenGroup (h, -1, 0, NULL);
  SetGroupSpacing (g, 10, 20);
  stp->staff = CreateStaffDialog (g, &(sfp->tables));
  stp->towplane = CreateMainTowDialog (g, &(sfp->tables));
  stp->field = CreateAirportDialog (g, &(sfp->tables));
  x = HiddenGroup (g, -5, 0, NULL);
  StaticPrompt (x, "Date", 0, popupMenuHeight, programFont, 'l');
  stp->date = CreateCalendarDateDialog (x, (Pointer) sfp, ChangeDate);
#ifdef WIN_MSWIN
  y = HiddenGroup (g, 3, 0, NULL);
  StaticPrompt (y, "Flash Drive Letter", 0, popupMenuHeight, programFont, 'l');
  stp->flash_drive = PopupList (y, TRUE, NULL);
  InitEnumPopup (stp->flash_drive, flash_alist, NULL);
  SetEnumPopupByName (stp->flash_drive, flash_alist, "D");
#endif
  AlignObjects (ALIGN_CENTER, (HANDLE) stp->staff, (HANDLE) stp->towplane,
                (HANDLE) stp->field, (HANDLE) x, (HANDLE) y, NULL);
  Hide (g);
  spp->pages [page] = g;
  page++;

  if (usesTracon) {
    g = HiddenGroup (h, -1, 0, NULL);
    stp->checklist = CreateChecklistDialog (g, &(sfp->tables));
    Hide (g);
    spp->pages [page] = g;
    page++;
  }

  g = HiddenGroup (h, -1, 0, NULL);
  sfp->helpDoc = CreateHelpDialog (g, wid);
  Hide (g);
  spp->pages [page] = g;
  page++;

  spp->pages [page] = NULL;

  spp->currPage = 0;

  Show (spp->pages [spp->currPage]);

  AlignObjects (ALIGN_CENTER, (HANDLE) tbs,
                (HANDLE) spp->pages [0],
                (HANDLE) spp->pages [1],
                (HANDLE) spp->pages [2],
                NULL);
}

/*
#define ADD_INTRO_PAGE      0
#define ADD_FAST_PAGE       1
#define ADD_SPECIAL_PAGE    2
#define ADD_TRANSIENT_PAGE  3
#define ADD_FULL_PAGE       4
#define ADD_GUEST_PAGE      5
#define ADD_PAYER_PAGE      6
#define ADD_STAFF_PAGE      7

static CharPtr personnelTitles [] = {
  "Intro",
  "FAST",
  "Visiting",
  "Transient",
  "Full",
  "Guest",
  "Payer",
  "Staff",
  NULL
};
*/

#define SHOW_CURRENT_PAGE   0
#define ADD_INTRO_PAGE      1
#define ADD_SPECIAL_PAGE    2
#define ADD_FULL_PAGE       3
#define ADD_GUEST_PAGE      4
#define ADD_PAYER_PAGE      5
#define ADD_STAFF_PAGE      6

static CharPtr personnelTitles [] = {
  "Current",
  "Intro",
  "Special",
  "Full",
  "Guest",
  "Payer",
  "Staff",
  NULL
};

static void ChangePersonnelTabs (
  VoidPtr data,
  Int2 newval,
  Int2 oldval
)

{
  SscFormPtr  sfp;
  SubPagePtr  spp;

  sfp = (SscFormPtr) data;
  if (sfp == NULL) return;

  spp = &(sfp->personnelPage);

  SafeHide (spp->pages [oldval]);
  Update ();

  spp->currPage = newval;
  SafeShow (spp->pages [newval]);
  Update ();
}

static void CreatePersonnelGroup (
  GrouP p,
  SscFormPtr sfp
)

{
  GrouP         g, h;
  SubPagePtr    spp;
  SscTablesPtr  stp;
  DialoG        tbs;

  if (sfp == NULL) return;
  spp = &(sfp->personnelPage);
  stp = &(sfp->tables);

  tbs = CreateFolderTabs (p, personnelTitles, 0, 0, 0, PROG_FOLDER_TAB,
                          ChangePersonnelTabs, (Pointer) sfp);

  h = HiddenGroup (p, 0, 0, NULL);
  spp->content = h;
  SetGroupMargins (h, 10, 10);

  g = HiddenGroup (h, -1, 0, NULL);
  SetGroupSpacing (g, 10, 10);
  stp->intro_members = CreateCurrentIntroDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [SHOW_CURRENT_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  SetGroupSpacing (g, 10, 10);
  CreateAddIntroDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [ADD_INTRO_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  SetGroupSpacing (g, 10, 10);
  CreateAddSpecialDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [ADD_SPECIAL_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  SetGroupSpacing (g, 10, 10);
  CreateAddFullDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [ADD_FULL_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  SetGroupSpacing (g, 10, 10);
  stp->guest_members = CreateAddGuestDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [ADD_GUEST_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  SetGroupSpacing (g, 10, 10);
  CreateAddPayerDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [ADD_PAYER_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  SetGroupSpacing (g, 10, 10);
  CreateAddStaffDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [ADD_STAFF_PAGE] = g;


  spp->currPage = 0;

  Show (spp->pages [spp->currPage]);

  AlignObjects (ALIGN_CENTER, (HANDLE) tbs,
                (HANDLE) spp->pages [SHOW_CURRENT_PAGE],
                (HANDLE) spp->pages [ADD_INTRO_PAGE],
                (HANDLE) spp->pages [ADD_SPECIAL_PAGE],
                (HANDLE) spp->pages [ADD_FULL_PAGE],
                (HANDLE) spp->pages [ADD_GUEST_PAGE],
                (HANDLE) spp->pages [ADD_PAYER_PAGE],
                (HANDLE) spp->pages [ADD_STAFF_PAGE],
                NULL);
}

#define GLIDER_FLIGHTS_PAGE   0
#define AIRPLANE_FLIGHTS_PAGE 1
#define PILOT_SUMMARY_PAGE    2
#define INSTR_SUMMARY_PAGE    3
#define TOW_QUEUE_PAGE        4

static CharPtr operationsTitles [] = {
  "Glider Flights",
  "Airplane Flights",
  "Pilot Summary",
  "Instructor Summary",
  "Tow Queue",
  NULL
};

static void ChangeOperationsTabs (
  VoidPtr data,
  Int2 newval,
  Int2 oldval
)

{
  SscFormPtr    sfp;
  SubPagePtr    spp;
  SscTablesPtr  stp;

  sfp = (SscFormPtr) data;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  spp = &(sfp->operationsPage);

  SafeHide (spp->pages [oldval]);
  Update ();

  if (newval == PILOT_SUMMARY_PAGE) {
    RecalculatePilotSummary (stp->glider_flights, stp->pilotsummary);
  }
  if (newval == INSTR_SUMMARY_PAGE) {
    RecalculateInstrSummary (stp->glider_flights, stp->instrsummary);
  }

  spp->currPage = newval;
  SafeShow (spp->pages [newval]);
  Update ();
}

static void CreateOperationsGroup (
  GrouP p,
  SscFormPtr sfp
)

{
  GrouP         g, h;
  SubPagePtr    spp;
  SscTablesPtr  stp;
  DialoG        tbs;

  if (sfp == NULL) return;
  spp = &(sfp->operationsPage);
  stp = &(sfp->tables);

  tbs = CreateFolderTabs (p, operationsTitles, 0, 0, 0, PROG_FOLDER_TAB,
                          ChangeOperationsTabs, (Pointer) sfp);

  h = HiddenGroup (p, 0, 0, NULL);
  spp->content = h;
  SetGroupMargins (h, 10, 10);

  g = HiddenGroup (h, -1, 0, NULL);
  stp->glider_flights = CreateGliderFlightsDialog (g, &(sfp->tables), (BaseFormPtr) sfp);
  Hide (g);
  spp->pages [GLIDER_FLIGHTS_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  StaticPrompt (g, "(Not yet implemented)", 0, dialogTextHeight, programFont, 'c');
  Hide (g);
  spp->pages [AIRPLANE_FLIGHTS_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  stp->pilotsummary = CreatePilotSummaryDialog (g, &(sfp->tables), (BaseFormPtr) sfp);
  Hide (g);
  spp->pages [PILOT_SUMMARY_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  stp->instrsummary = CreateInstrSummaryDialog (g, &(sfp->tables), (BaseFormPtr) sfp);
  Hide (g);
  spp->pages [INSTR_SUMMARY_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  StaticPrompt (g, "(Not yet implemented)", 0, dialogTextHeight, programFont, 'c');
  Hide (g);
  spp->pages [TOW_QUEUE_PAGE] = g;

  spp->currPage = 0;

  Show (spp->pages [spp->currPage]);

  AlignObjects (ALIGN_CENTER, (HANDLE) tbs,
                (HANDLE) spp->pages [GLIDER_FLIGHTS_PAGE],
                (HANDLE) spp->pages [AIRPLANE_FLIGHTS_PAGE],
                (HANDLE) spp->pages [PILOT_SUMMARY_PAGE],
                (HANDLE) spp->pages [INSTR_SUMMARY_PAGE],
                (HANDLE) spp->pages [TOW_QUEUE_PAGE],
                NULL);
}

#define PAYMENTS_PAGE     0
#define FEES_PAGE         1
#define CHARGES_PAGE      2
#define ADJUSTMENTS_PAGE  3
#define EXPENSES_PAGE     4

static CharPtr financesTitles [] = {
  "Payments",
  "Fees",
  "Charges",
  "Adjustments",
  "Expenses",
  NULL
};

static void ChangeFinancesTabs (
  VoidPtr data,
  Int2 newval,
  Int2 oldval
)

{
  SscFormPtr    sfp;
  SubPagePtr    spp;
  SscTablesPtr  stp;

  sfp = (SscFormPtr) data;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  spp = &(sfp->financesPage);

  SafeHide (spp->pages [oldval]);
  Update ();

  if (newval == PAYMENTS_PAGE) {
    RecalculateCharges (stp->glider_flights, stp->fees, stp->charges, stp->adjustments, stp->payments);
  }
  spp->currPage = newval;
  SafeShow (spp->pages [newval]);
  Update ();
}

static void CreateFinancesGroup (
  GrouP p,
  SscFormPtr sfp
)

{
  GrouP         g, h;
  SubPagePtr    spp;
  SscTablesPtr  stp;
  DialoG        tbs;

  if (sfp == NULL) return;
  spp = &(sfp->financesPage);
  stp = &(sfp->tables);

  tbs = CreateFolderTabs (p, financesTitles, 0, 0, 0, PROG_FOLDER_TAB,
                          ChangeFinancesTabs, (Pointer) sfp);

  h = HiddenGroup (p, 0, 0, NULL);
  spp->content = h;
  SetGroupMargins (h, 10, 10);

  g = HiddenGroup (h, -1, 0, NULL);
  stp->payments = CreatePaymentsDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [PAYMENTS_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  stp->fees = CreateFeeDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [FEES_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  stp->charges = CreateChargeDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [CHARGES_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  stp->adjustments = CreateAdjustmentsDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [ADJUSTMENTS_PAGE] = g;


  g = HiddenGroup (h, 1, 0, NULL);
  stp->expenses = CreateAddExpensesDialog (g, &(sfp->tables));
  Hide (g);
  spp->pages [EXPENSES_PAGE] = g;

  spp->currPage = 0;

  Show (spp->pages [spp->currPage]);

  AlignObjects (ALIGN_CENTER, (HANDLE) tbs,
                (HANDLE) spp->pages [PAYMENTS_PAGE],
                (HANDLE) spp->pages [FEES_PAGE],
                (HANDLE) spp->pages [CHARGES_PAGE],
                (HANDLE) spp->pages [ADJUSTMENTS_PAGE],
                (HANDLE) spp->pages [EXPENSES_PAGE],
                NULL);
}

#define TOWPLANE_PAGE  0
#define CONTACTS_PAGE  1
#define BADGE_PAGE     2
#define COMMENT_PAGE   3
#define NO_OPS_PAGE    4

static CharPtr shutdownTitles [] = {
  "Tow Plane Data",
  "Contact Info",
  "Awards",
  "Comment",
  "No Operations",
  NULL
};

static void ChangeShutdownTabs (
  VoidPtr data,
  Int2 newval,
  Int2 oldval
)

{
  SscFormPtr  sfp;
  SubPagePtr  spp;

  sfp = (SscFormPtr) data;
  if (sfp == NULL) return;

  spp = &(sfp->shutdownPage);

  SafeHide (spp->pages [oldval]);
  Update ();

  spp->currPage = newval;
  SafeShow (spp->pages [newval]);
  Update ();
}

static void CreateShutdownGroup (
  GrouP p,
  SscFormPtr sfp
)

{
  GrouP         g, h;
  SubPagePtr    spp;
  SscTablesPtr  stp;
  DialoG        tbs;

  if (sfp == NULL) return;
  spp = &(sfp->shutdownPage);
  stp = &(sfp->tables);

  tbs = CreateFolderTabs (p, shutdownTitles, 0, 0, 0, PROG_FOLDER_TAB,
                          ChangeShutdownTabs, (Pointer) sfp);

  h = HiddenGroup (p, 0, 0, NULL);
  spp->content = h;
  SetGroupMargins (h, 10, 10);

  g = HiddenGroup (h, -1, 0, NULL);
  stp->towplane_data = CreateTowplaneDialog (g, stp);
  Hide (g);
  spp->pages [TOWPLANE_PAGE] = g;

  g = HiddenGroup (h, 1, 0, NULL);
  /*
  StaticPrompt (g, "New Members", 0, dialogTextHeight, programFont, 'c');
  */
  stp->new_contacts = CreateContactDialog (g, stp);
  Hide (g);
  spp->pages [CONTACTS_PAGE] = g;

  g = HiddenGroup (h, 1, 0, NULL);
  stp->awards = CreateBadgesDialog (g, stp);
  Hide (g);
  spp->pages [BADGE_PAGE] = g;

  g = HiddenGroup (h, 1, 0, NULL);
  StaticPrompt (g, "General Comment", 0, dialogTextHeight, programFont, 'c');
  stp->comment = ScrollText (g, 20, 5, programFont, TRUE, NULL);
  Hide (g);
  spp->pages [COMMENT_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  stp->no_ops = CheckBox (g, "No Flight Operations", NULL);
  Hide (g);
  spp->pages [NO_OPS_PAGE] = g;

  spp->currPage = 0;

  Show (spp->pages [spp->currPage]);

  AlignObjects (ALIGN_CENTER, (HANDLE) tbs,
                (HANDLE) spp->pages [TOWPLANE_PAGE],
                (HANDLE) spp->pages [CONTACTS_PAGE],
                (HANDLE) spp->pages [BADGE_PAGE],
                (HANDLE) spp->pages [COMMENT_PAGE],
                (HANDLE) spp->pages [NO_OPS_PAGE],
                NULL);
}

#define STARTUP_PAGE     0
#define PERSONNEL_PAGE   1
#define OPERATIONS_PAGE  2
#define FINANCES_PAGE    3
#define SHUTDOWN_PAGE    4

static CharPtr mainTitles [] = {
  "STARTUP",
  "MEMBERS",
  "OPERATIONS",
  "FINANCES",
  "SHUTDOWN",
  NULL
};

static void ChangeMainTabs (
  VoidPtr data,
  Int2 newval,
  Int2 oldval
)

{
  SscFormPtr    sfp;
  SubPagePtr    spp;
  SscTablesPtr  stp;

  sfp = (SscFormPtr) data;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  spp = &(sfp->mainPage);

  SafeHide (spp->pages [oldval]);
  Update ();

  if (newval == OPERATIONS_PAGE) {
    Enable (sfp->flightMenu);
  } else {
    Disable (sfp->flightMenu);
  }
  if (newval == FINANCES_PAGE) {
    Enable (sfp->paymentMenu);
  } else {
    Disable (sfp->paymentMenu);
  }
  if (newval == PERSONNEL_PAGE) {
    Enable (sfp->personMenu);
  } else {
    Disable (sfp->personMenu);
  }

  if (newval == FINANCES_PAGE) {
    RecalculateCharges (stp->glider_flights, stp->fees, stp->charges, stp->adjustments, stp->payments);
  }

  spp->currPage = newval;
  SafeShow (spp->pages [newval]);
  Update ();
}

static CharPtr dirtymessage = "Current log not saved - do you wish to overwrite it";

static void OpenLogSheet (
  SscFormPtr sfp,
  LogSheetPtr lsp
)

{
  AdjustDataPtr    adjustments, fap;
  Char             buf [64];
  CalendarDatePtr  cdp;
  ChargeDataPtr    charges, mcp;
  FeeDataPtr       fees, fdp;
  GliderFlightPtr  fip, flights;
  StaffDataPtr     sdp;
  SscTablesPtr     stp;
  Char             ttl [128];

  if (sfp == NULL || lsp == NULL) return;
  stp = &(sfp->tables);

  cdp = lsp->date;
  if (cdp != NULL) {
    stp->currentDate.month = cdp->month;
    stp->currentDate.day = cdp->day;
    stp->currentDate.year = cdp->year;
    PrintCalendarDate (cdp, buf, DATE_WITH_DAY);
  } else {
    StringCpy (buf, "?");
  }
  StringNCpy (stp->date_string, buf, sizeof (stp->date_string));

  sprintf (ttl, "Skyline Soaring Logsheet %s - %s", program_version, buf);
  SetTitle (sfp->form, ttl);

  ClearDirtyFlags (stp);

  sdp = lsp->staff;
  if (sdp != NULL) {
    AddNameToExtras (&(stp->dutyofficers), sdp->dutyofficer);
    AddNameToExtras (&(stp->assistants), sdp->assistant);
    AddNameToExtras (&(stp->members), sdp->timekeeper);
    AddNameToExtras (&(stp->instructors), sdp->instructor);
    AddNameToExtras (&(stp->justcfigs), sdp->instructor);
    AddNameToExtras (&(stp->towpilots), sdp->towpilot);
    AddNameToExtras (&(stp->towpilots), sdp->am_towpilot);
    AddNameToExtras (&(stp->towpilots), sdp->pm_towpilot);
  }

  flights = lsp->glider_flights;
  for (fip = flights; fip != NULL; fip = fip->next) {
    AddNameToExtras (&(stp->members), fip->name);
    AddNameToExtras (&(stp->instructors), fip->instructor);
    AddNameToExtras (&(stp->justcfigs), fip->instructor);
    AddNameToExtras (&(stp->towpilots), fip->towpilot);
    AddNameToExtras (&(stp->payers), fip->name);
    AddNameToExtras (&(stp->payers), fip->alt_payer);
  }

  fees = lsp->fees;
  for (fdp = fees; fdp != NULL; fdp = fdp->next) {
    AddNameToExtras (&(stp->payers), fdp->name);
  }

  charges = lsp->charges;
  for (mcp = charges; mcp != NULL; mcp = mcp->next) {
    AddNameToExtras (&(stp->payers), mcp->name);
  }

  adjustments = lsp->adjustments;
  for (fap = adjustments; fap != NULL; fap = fap->next) {
    AddNameToExtras (&(stp->payers), fap->name);
  }

  Hide (sfp->startupPage.content);
  Hide (sfp->personnelPage.content);
  Hide (sfp->operationsPage.content);
  Hide (sfp->financesPage.content);
  Hide (sfp->shutdownPage.content);
  Update ();

  UpdateStaffControls (stp->staff);
  UpdateFlightControls (stp->glider_flights);
  UpdateFeesControls (stp->fees);
  UpdateChargesControls (stp->charges);
  UpdateAdjustmentsControls (stp->adjustments);
  UpdateBadgesControls (stp->awards);

  ClearDirtyFlags (stp);

  PointerToDialog (stp->date, lsp->date);
  PointerToDialog (stp->towplane, lsp->towplane);
  PointerToDialog (stp->field, lsp->airport);
  SetStatus (stp->no_ops, (Boolean) (lsp->no_operations != 0));
  PointerToDialog (stp->staff, lsp->staff);
  PointerToDialog (stp->glider_flights, lsp->glider_flights);
  PointerToDialog (stp->pilotsummary, lsp->glider_flights);
  PointerToDialog (stp->instrsummary, lsp->glider_flights);
  PointerToDialog (stp->payments, lsp->payments);
  PointerToDialog (stp->fees, lsp->fees);
  PointerToDialog (stp->charges, lsp->charges);
  PointerToDialog (stp->adjustments, lsp->adjustments);
  PointerToDialog (stp->towplane_data, lsp->towplane_data);
  PointerToDialog (stp->new_contacts, lsp->new_contacts);
  PointerToDialog (stp->awards, lsp->awards);
  PointerToDialog (stp->expenses, lsp->expenses);
  SafeSetTitle (stp->comment, lsp->comment);

  LogSheetFree (lsp);

  Show (sfp->startupPage.content);
  Show (sfp->personnelPage.content);
  Show (sfp->operationsPage.content);
  Show (sfp->financesPage.content);
  Show (sfp->shutdownPage.content);
  Update ();

  stp->need_to_update = FALSE;
  stp->need_to_auto_save = FALSE;
  stp->auto_delay = 0;
}

static void OpenLogProc (IteM i)

{
  AsnIoPtr         aip;
  MsgAnswer        ans;
  BaseFormPtr      bfp;
  LogSheetPtr      lsp;
  Char             path [PATH_MAX];
  SscFormPtr       sfp;
  SscTablesPtr     stp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  if (stp->need_to_update) {
    ans = Message (MSG_OKC, "%s", dirtymessage);
    if (ans == ANS_CANCEL) return;
  }

  lsp = NULL;
  if (GetInputFileName (path, sizeof (path), NULL, "TEXT")) {
    aip = AsnIoOpen (path, "r");
    if (aip != NULL) {
      lsp = LogSheetAsnRead (aip, NULL);
      AsnIoClose (aip);
    }
  }
  if (lsp == NULL) return;

  ModernizeLogSheet (lsp);

  OpenLogSheet (sfp, lsp);
}

static Boolean HasSubdirectory (
  CharPtr dir,
  CharPtr sub
)

{
  Boolean     found;
  ValNodePtr  head;
  CharPtr     str;
  ValNodePtr  vnp;

  if (StringHasNoText (dir) || StringHasNoText (sub)) return FALSE;

  found = FALSE;
  head = DirCatalog (dir);
  for (vnp = head; vnp != NULL; vnp = vnp->next) {
    if (vnp->choice != 1) continue;
    str = (CharPtr) vnp->data.ptrvalue;
    if (StringHasNoText (str)) continue;
    if (StringICmp (str, sub) == 0) {
      found = TRUE;
    }
  }
  ValNodeFreeData (head);

  return found;
}

static void CheckForExistingLogFile (
  SscFormPtr sfp
)

{
  AsnIoPtr         aip;
  MsgAnswer        ans;
  Char             buf [32];
  CalendarDatePtr  cdp;
  LogSheetPtr      lsp;
  Char             path [PATH_MAX];
  CharPtr          ptr;
  SscTablesPtr     stp;
  Char             year [8];

  if (sfp == NULL) return;
  stp = &(sfp->tables);

  PrintCalendarDate (&(stp->currentDate), buf, DATE_FOR_FILE);
  StringCat (buf, ".txt");
  sprintf (year, "%ld", (long) stp->currentDate.year);
  cdp = DialogToPointer (stp->date);
  if (cdp != NULL) {
    PrintCalendarDate (cdp, buf, DATE_FOR_FILE);
    StringCat (buf, ".txt");
    sprintf (year, "%ld", (long) cdp->year);
    cdp = CalendarDateFree (cdp);
  }
  if (buf [0] == ' ') {
    buf [0] = '0';
  }
  if (buf [2] == ' ') {
    buf [2] = '0';
  }

  ProgramPath (path, sizeof (path));
  ptr = StringRChr (path, DIRDELIMCHR);
  if (ptr != NULL) {
    *ptr = '\0';
  }

#ifdef WIN_MAC
  ptr = StringStr (path, "/ssc/build");
  if (ptr != NULL) {
    ptr = StringStr (path, "build");
    if (ptr != NULL) {
      *ptr = '\0';
    }
  }
#endif

  if (HasSubdirectory (path, "logs")) {
    FileBuildPath (path, "logs", NULL);
    if (HasSubdirectory (path, year)) {
      FileBuildPath (path, year, NULL);
    }
  }

  FileBuildPath (path, NULL, buf);

  if (FileLength (path) < 4) return;

  ans = Message (MSG_YN, "Do you want to open the existing log file for %s?", buf);
  if (ans == ANS_NO) return;

  lsp = NULL;
  aip = AsnIoOpen (path, "r");
  if (aip != NULL) {
    lsp = LogSheetAsnRead (aip, NULL);
    AsnIoClose (aip);
  }
  if (lsp == NULL) {
    Message (MSG_OK, "Unable to read existing log file %s?", buf);
    return;
  }

  ModernizeLogSheet (lsp);

  OpenLogSheet (sfp, lsp);
}

static Pointer SscFormToLogSheetPtr (
  ForM f
)

{
  LogSheetPtr   lsp;
  SscFormPtr    sfp;
  SscTablesPtr  stp;

  sfp = (SscFormPtr) GetObjectExtra (f);
  if (sfp == NULL) return NULL;
  stp = &(sfp->tables);
  if (stp == NULL) return NULL;

  lsp = LogSheetNew ();
  if (lsp == NULL) return NULL;

  lsp->date = DialogToPointer (stp->date);
  lsp->version = StringSaveNoNull (data_spec_version);
  lsp->club = StringSaveNoNull (stp->club);
  lsp->airport = DialogToPointer (stp->field);
  lsp->towplane = DialogToPointer (stp->towplane);
  lsp->no_operations = (Uint1) GetStatus (stp->no_ops);
  lsp->staff = DialogToPointer (stp->staff);
  lsp->glider_flights = DialogToPointer (stp->glider_flights);
  lsp->payments = DialogToPointer (stp->payments);
  lsp->fees = DialogToPointer (stp->fees);
  lsp->charges = DialogToPointer (stp->charges);
  lsp->adjustments = DialogToPointer (stp->adjustments);
  lsp->towplane_data = DialogToPointer (stp->towplane_data);
  lsp->new_contacts = DialogToPointer (stp->new_contacts);
  lsp->awards = DialogToPointer (stp->awards);
  lsp->expenses = DialogToPointer (stp->expenses);
  lsp->comment = SaveStringFromText (stp->comment);
  FixNonPrintableChars (lsp->expenses);

  return (Pointer) lsp;
}

#define INSTRUCTING_SELF            1
#define TOWING_SELF                 2
#define INSTRUCTOR_TOWS             4
#define INSTRUCTOR_MISSING          8
#define INCONSISTENT_CLASS         16
#define TOW_PILOT_MISSING          32
#define GLIDER_MISSING             64
#define RELEASE_HEIGHT_MISSING    128
#define TAKEOFF_TIME_MISSING      256
#define LANDING_TIME_MISSING      512
#define PAYMENT_PROBLEM          1024
#define HOME_PHONE_MISSING       2048
#define E_MAIL_MISSING           4096
#define BAD_TACH_TIME            8192
#define AIRPORT_MISSING         16384
#define BAD_GAS_ADDED           32768

static Int4 ValidateLogsheet (
  SscFormPtr sfp,
  Boolean final
)

{
  DollarAmount     amount_paid;
  ContactInfoPtr   cip;
  ClockTimePtr     ctp;
  DollarAmountPtr  dap;
  GliderFlightPtr  gfp;
  LogSheetPtr      lsp;
  PaymentInfoPtr   pip;
  Int4             result = 0;
  CharPtr          str;
  TowPlaneDataPtr  tdp;

  if (sfp == NULL) return 0;

  lsp = (LogSheetPtr) FormToPointer (sfp->form);
  if (lsp == NULL) return 0;

  for (gfp = lsp->glider_flights; gfp != NULL; gfp = gfp->next) {
    if (StringDoesHaveText (gfp->name)) {
      if (StringICmp (gfp->name, gfp->instructor) == 0) {
        result |= INSTRUCTING_SELF;
      }
      if (StringICmp (gfp->name, gfp->towpilot) == 0) {
        result |= TOWING_SELF;
      }
      if (StringDoesHaveText (gfp->instructor) &&
          StringICmp (gfp->instructor, gfp->towpilot) == 0) {
        result |= INSTRUCTOR_TOWS;
      }
      if (StringHasNoText (gfp->instructor) && gfp->category == 2) {
        result |= INSTRUCTOR_MISSING;
      }
      if (StringDoesHaveText (gfp->instructor)) {
        if (gfp->category == 1) {
          result |= INCONSISTENT_CLASS;
        }
      }
      if (StringHasNoText (gfp->towpilot)) {
        result |= TOW_PILOT_MISSING;
      }
    }

    ctp = gfp->takeoff;
    if (ctp != NULL && ctp->hour != -1 && ctp->minute != -1) {
      if (StringHasNoText (gfp->glider)) {
        result |= GLIDER_MISSING;
      }
    }

    if (gfp->release_altitude <= 0) {
      if (gfp->exception != 2 && gfp->exception != 5) {
        result |= RELEASE_HEIGHT_MISSING;
      }
    }

    ctp = gfp->landing;
    if (final) {
      if (ctp == NULL || ctp->hour == -1 || ctp->minute == -1) {
        result |= LANDING_TIME_MISSING;
      }
    }
    if (ctp != NULL && ctp->hour != -1 && ctp->minute != -1) {
      ctp = gfp->takeoff;
      if (ctp == NULL || ctp->hour == -1 || ctp->minute == -1) {
        result |= TAKEOFF_TIME_MISSING;
      }
    }
  }

  if (final) {
    for (pip = lsp->payments; pip != NULL; pip = pip->next) {
      if (StringDoesHaveText (pip->payer)) {
        dap = pip->charges;
        if (dap != NULL && dap->dollars >= 0 && dap->cents >= 0) {
          SumPayments (pip->check_payment, pip->cash_payment, pip->account_payment,
                       pip->voucher_payment, &amount_paid);
          if (amount_paid.dollars != dap->dollars || amount_paid.cents != dap->cents) {
            result |= PAYMENT_PROBLEM;
          }
        }
      }
    }
  } else {
    for (pip = lsp->payments; pip != NULL; pip = pip->next) {
      if (StringDoesHaveText (pip->payer)) {
        dap = pip->charges;
        if (dap != NULL && dap->dollars >= 0 && dap->cents >= 0) {
          SumPayments (pip->check_payment, pip->cash_payment, pip->account_payment,
                       pip->voucher_payment, &amount_paid);
          if (amount_paid.dollars != dap->dollars || amount_paid.cents != dap->cents) {
            if (amount_paid.dollars != 0 || amount_paid.cents != 0) {
              result |= PAYMENT_PROBLEM;
            }
          }
        }
      }
    }
  }

  for (cip = lsp->new_contacts; cip != NULL; cip = cip->next) {
    if (StringHasNoText (cip->home_phone)) {
      result |= HOME_PHONE_MISSING;
    }
    if (StringHasNoText (cip->e_mail)) {
      result |= E_MAIL_MISSING;
    }
  }

  for (tdp = lsp->towplane_data; tdp != NULL; tdp = tdp->next) {

    str = tdp->start_tach;
    if (   StringDoesHaveText (str)
        && (StringChr (str, '.') == NULL || !IS_DIGIT(*str) || !IS_DIGIT(*(str + strlen(str) - 1)))
       ) {
      result |= BAD_TACH_TIME;
    }

    str = tdp->end_tach;
    if (    StringDoesHaveText (str)
         && (StringChr (str, '.') == NULL || !IS_DIGIT(*str) || !IS_DIGIT(*(str + strlen(str) - 1)))
       ) {
      result |= BAD_TACH_TIME;
    }

    str = tdp->gas_added;
    if (    StringDoesHaveText (str)
         && (StringChr (str, '.') == NULL || !IS_DIGIT(*str) || !IS_DIGIT(*(str + strlen(str) - 1)))
       ) {
      result |= BAD_GAS_ADDED;
    }
  }

  if (StringHasNoText (lsp->airport)) {
    result |= AIRPORT_MISSING;
  }

  LogSheetFree (lsp);

  return result;
}

static void SaveToFileProc (
  SscFormPtr sfp,
  CharPtr path,
  CharPtr altpath,
  CharPtr mode,
  Boolean recalculate_charges,
  Boolean expand
)

{
  AsnIoPtr      aip;
  LogSheetPtr   lsp;
  SscTablesPtr  stp;

  if (sfp == NULL) return;
  stp = &(sfp->tables);
  if (stp == NULL) return;

  if (recalculate_charges) {
    RecalculateCharges (stp->glider_flights, stp->fees, stp->charges, stp->adjustments, stp->payments);
  }

  lsp = (LogSheetPtr) FormToPointer (sfp->form);
  if (lsp == NULL) {
    Message (MSG_OK, "Unable to allocate logsheet");
    return;
  }

  WatchCursor ();

  if (StringDoesHaveText (path)) {
    aip = AsnIoOpen (path, mode);
#ifdef WIN_MAC
    if (aip == NULL && StringDoesHaveText (altpath)) {
      aip = AsnIoOpen (altpath, mode);
    }
#endif
    if (aip != NULL) {
      ContractLogSheet (lsp);
      if (expand) {
        ExpandLogSheet (lsp);
      }
      if (ValidateLogsheet (sfp, TRUE) == 0) {
        lsp->passed_validation = TRUE;
      }
      if (LogSheetAsnWrite (lsp, aip, NULL)) {
        stp->need_to_update = FALSE;
      } else {
        ArrowCursor ();
        Message (MSG_OK, "LogSheet writing failed");
      }
      AsnIoClose (aip);
    } else {
      ArrowCursor ();
      Message (MSG_OK, "Could not create output file");
    }
  }

  LogSheetFree (lsp);

  ArrowCursor ();
}

static void DoSaveLogProc (SscFormPtr sfp)

{
  Char             altpath [PATH_MAX];
  Char             buf [32];
  CalendarDatePtr  cdp;
  Boolean          hasLogs;
  Char             path [PATH_MAX];
  CharPtr          ptr;
  SscTablesPtr     stp;
  Char             year [8];

  if (sfp == NULL) return;
  stp = &(sfp->tables);

  PrintCalendarDate (&(stp->currentDate), buf, DATE_FOR_FILE);
  StringCat (buf, ".txt");
  sprintf (year, "%ld", (long) stp->currentDate.year);
  cdp = DialogToPointer (stp->date);
  if (cdp != NULL) {
    PrintCalendarDate (cdp, buf, DATE_FOR_FILE);
    StringCat (buf, ".txt");
    sprintf (year, "%ld", (long) cdp->year);
    cdp = CalendarDateFree (cdp);
  }
  if (buf [0] == ' ') {
    buf [0] = '0';
  }
  if (buf [2] == ' ') {
    buf [2] = '0';
  }

  ProgramPath (path, sizeof (path));
  ptr = StringRChr (path, DIRDELIMCHR);
  if (ptr != NULL) {
    *ptr = '\0';
  }

#ifdef WIN_MAC
  ptr = StringStr (path, "/ssc/build");
  if (ptr != NULL) {
    ptr = StringStr (path, "build");
    if (ptr != NULL) {
      *ptr = '\0';
    }
  }
#endif

  hasLogs = FALSE;
  if (HasSubdirectory (path, "logs")) {
    hasLogs = TRUE;
    FileBuildPath (path, "logs", NULL);
    if (HasSubdirectory (path, year)) {
      FileBuildPath (path, year, NULL);
    }
  }

  FileBuildPath (path, NULL, buf);

  altpath [0] = '\0';
#ifdef WIN_MAC
  if (! hasLogs) {
    ProgramPath (altpath, sizeof (path));
    ptr = StringStr (altpath, "ncbi");
    if (ptr != NULL) {
      *ptr = '\0';
    }
    FileBuildPath (altpath, "Misc", NULL);
    FileBuildPath (altpath, "ssc", NULL);

    if (HasSubdirectory (altpath, "logs")) {
      hasLogs = TRUE;
      FileBuildPath (altpath, "logs", NULL);
    }

    FileBuildPath (altpath, NULL, buf);
  }
#endif

  if (! hasLogs) {
    Beep ();
    Message (MSG_OK, "Cannot find logs directory, file not saved");
    return;
  }

  SaveToFileProc (sfp, path, altpath, "w", TRUE, FALSE);
}

static void SaveLogProc (IteM i)

{
  BaseFormPtr  bfp;
  SscFormPtr   sfp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  DoSaveLogProc (sfp);
}

static void SaveAsLogProc (IteM i)

{
  MsgAnswer        ans;
  BaseFormPtr      bfp;
  Char             buf [32];
  CalendarDatePtr  cdp;
  Char             path [PATH_MAX];
  SscFormPtr       sfp;
  SscTablesPtr     stp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  PrintCalendarDate (&(stp->currentDate), buf, DATE_FOR_FILE);
  StringCat (buf, ".txt");
  cdp = DialogToPointer (stp->date);
  if (cdp != NULL) {
    PrintCalendarDate (cdp, buf, DATE_FOR_FILE);
    StringCat (buf, ".txt");
    cdp = CalendarDateFree (cdp);
  }
  if (buf [0] == ' ') {
    buf [0] = '0';
  }
  if (buf [2] == ' ') {
    buf [2] = '0';
  }

  ans = Message (MSG_OKC, "Use Save to Hard Disk for normal use. Do you wish to continue with Save As?");
  if (ans == ANS_NO || ans == ANS_CANCEL) return;

  if (GetOutputFileName (path, sizeof (path), buf)) {
    SaveToFileProc (sfp, path, NULL, "w", TRUE, FALSE);
  }
}

static void SaveFloppyLogProc (IteM i)

{
  BaseFormPtr      bfp;
  Char             buf [32];
  CalendarDatePtr  cdp;
  Char             path [PATH_MAX];
  SscFormPtr       sfp;
  SscTablesPtr     stp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  PrintCalendarDate (&(stp->currentDate), buf, DATE_FOR_FILE);
  StringCat (buf, ".txt");
  cdp = DialogToPointer (stp->date);
  if (cdp != NULL) {
    PrintCalendarDate (cdp, buf, DATE_FOR_FILE);
    StringCat (buf, ".txt");
    cdp = CalendarDateFree (cdp);
  }
  if (buf [0] == ' ') {
    buf [0] = '0';
  }
  if (buf [2] == ' ') {
    buf [2] = '0';
  }

#ifdef WIN_MSWIN
  StringCpy (path, "A:\\");
  StringCat (path, buf);

  SaveToFileProc (sfp, path, NULL, "w", TRUE, FALSE);
#else
  if (GetOutputFileName (path, sizeof (path), buf)) {
    SaveToFileProc (sfp, path, NULL, "w", TRUE, FALSE);
  }
#endif
}

static void SaveFlashLogProc (IteM i)

{
  BaseFormPtr      bfp;
  Char             buf [32];
  CalendarDatePtr  cdp;
  Char             path [PATH_MAX];
  SscFormPtr       sfp;
  SscTablesPtr     stp;
#ifdef WIN_MSWIN
  CharPtr          drive;
#endif

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  PrintCalendarDate (&(stp->currentDate), buf, DATE_FOR_FILE);
  StringCat (buf, ".txt");
  cdp = DialogToPointer (stp->date);
  if (cdp != NULL) {
    PrintCalendarDate (cdp, buf, DATE_FOR_FILE);
    StringCat (buf, ".txt");
    cdp = CalendarDateFree (cdp);
  }
  if (buf [0] == ' ') {
    buf [0] = '0';
  }
  if (buf [2] == ' ') {
    buf [2] = '0';
  }

#ifdef WIN_MSWIN
  drive = GetEnumPopupByName (stp->flash_drive, flash_alist);
  StringCpy (path, drive);
  StringCat (path, ":\\");
  StringCat (path, buf);

  SaveToFileProc (sfp, path, NULL, "w", TRUE, FALSE);
#else
  if (GetOutputFileName (path, sizeof (path), buf)) {
    SaveToFileProc (sfp, path, NULL, "w", TRUE, FALSE);
  }
#endif
}

static void SaveAsXMLProc (IteM i)

{
  BaseFormPtr      bfp;
  Char             buf [32];
  CalendarDatePtr  cdp;
  Char             path [PATH_MAX];
  SscFormPtr       sfp;
  SscTablesPtr     stp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  PrintCalendarDate (&(stp->currentDate), buf, DATE_FOR_FILE);
  StringCat (buf, ".xml");
  cdp = DialogToPointer (stp->date);
  if (cdp != NULL) {
    PrintCalendarDate (cdp, buf, DATE_FOR_FILE);
    StringCat (buf, ".xml");
    cdp = CalendarDateFree (cdp);
  }
  if (buf [0] == ' ') {
    buf [0] = '0';
  }
  if (buf [2] == ' ') {
    buf [2] = '0';
  }

  if (GetOutputFileName (path, sizeof (path), buf)) {
    SaveToFileProc (sfp, path, NULL, "wx", TRUE, TRUE);
  }
}

static void SaveAutoProc (SscFormPtr sfp)

{
  Char             altpath [PATH_MAX];
  Char             buf [32];
  CalendarDatePtr  cdp;
  Boolean          hasLogs;
  Boolean          need_to_update;
  Char             path [PATH_MAX];
  CharPtr          ptr;
  SscTablesPtr     stp;
  Char             year [8];

  if (sfp == NULL) return;
  stp = &(sfp->tables);

  PrintCalendarDate (&(stp->currentDate), buf, DATE_FOR_FILE);
  StringCat (buf, ".txt");
  sprintf (year, "%ld", (long) stp->currentDate.year);
  cdp = DialogToPointer (stp->date);
  if (cdp != NULL) {
    PrintCalendarDate (cdp, buf, DATE_FOR_FILE);
    StringCat (buf, ".txt");
    sprintf (year, "%ld", (long) cdp->year);
    cdp = CalendarDateFree (cdp);
  }
  if (buf [0] == ' ') {
    buf [0] = '0';
  }
  if (buf [2] == ' ') {
    buf [2] = '0';
  }
  buf [4] = buf [6];
  buf [5] = buf [7];
  buf [6] = '_';
  (stp->auto_cycle)++;
  if (stp->auto_cycle > 9) {
    stp->auto_cycle = 1;
  }
  buf [7] = '0' + stp->auto_cycle;

  ProgramPath (path, sizeof (path));
  ptr = StringRChr (path, DIRDELIMCHR);
  if (ptr != NULL) {
    *ptr = '\0';
  }

#ifdef WIN_MAC
  ptr = StringStr (path, "/ssc/build");
  if (ptr != NULL) {
    ptr = StringStr (path, "build");
    if (ptr != NULL) {
      *ptr = '\0';
    }
  }
#endif

  hasLogs = FALSE;
  if (HasSubdirectory (path, "logs")) {
    hasLogs = TRUE;
    FileBuildPath (path, "logs", NULL);
    /*
    if (HasSubdirectory (path, year)) {
      FileBuildPath (path, year, NULL);
    }
    */
    if (HasSubdirectory (path, "auto")) {
      FileBuildPath (path, "auto", NULL);
    }
  }

  FileBuildPath (path, NULL, buf);

  altpath [0] = '\0';
#ifdef WIN_MAC
  if (! hasLogs) {
    ProgramPath (altpath, sizeof (path));
    ptr = StringStr (altpath, "ncbi");
    if (ptr != NULL) {
      *ptr = '\0';
    }
    FileBuildPath (altpath, "Misc", NULL);
    FileBuildPath (altpath, "ssc", NULL);

    if (HasSubdirectory (altpath, "logs")) {
      hasLogs = TRUE;
      FileBuildPath (altpath, "logs", NULL);
      /*
      if (HasSubdirectory (altpath, year)) {
        FileBuildPath (altpath, year, NULL);
      }
      */
      if (HasSubdirectory (altpath, "auto")) {
        FileBuildPath (altpath, "auto", NULL);
      }
    }

    FileBuildPath (altpath, NULL, buf);
  }
#endif

  if (! hasLogs) {
    Beep ();
    Message (MSG_POSTERR, "Cannot find auto logs directory, file not saved");
    return;
  }

  need_to_update = stp->need_to_update;
  SaveToFileProc (sfp, path, altpath, "w", FALSE, FALSE);
  if (need_to_update) {
    if (! stp->need_to_update) {
      DoSaveLogProc (sfp);
    }
  }
}

static CharPtr SaveNameString (
  CharPtr name,
  SscTablesPtr stp
)

{
  Char     buf [128], tmp [128];
  CharPtr  first, suffix;

  if (StringHasNoText (name)) return NULL;

  StringNCpy_0 (tmp, name, sizeof (tmp));
  TrimSpacesAroundString (tmp);
  if (tmp [0] == '<') return NULL;

  first = StringChr (tmp, ',');
  if (first == NULL) {
    return StringSave (tmp);
  }

  *first = '\0';
  first++;
  TrimSpacesAroundString (first);
  suffix = StringChr (first, ' ');
  if (suffix != NULL) {
    *suffix = '\0';
    suffix++;
  }

  StringCpy (buf, first);
  StringCat (buf, " ");
  StringCat (buf, tmp);
  if (suffix != NULL) {
    StringCat (buf, " ");
    StringCat (buf, suffix);
  }

  return StringSave (buf);
}

static CharPtr SaveGliderString (
  CharPtr glider
)

{
  if (StringHasNoText (glider)) return NULL;

  if (StringStr (glider, "ASK") != NULL ||
      StringStr (glider, "K-21") != NULL ||
      StringStr (glider, "K 21") != NULL) {
    return StringSave ("ASK-21");
  }
  if (StringStr (glider, "Grob") != NULL ||
      StringStr (glider, "GROB") != NULL ||
      StringStr (glider, "G-103") != NULL ||
      StringStr (glider, "G 103") != NULL) {
    return StringSave ("GROB 103");
  }
  if (StringStr (glider, "SGS") != NULL ||
      StringStr (glider, "1-36") != NULL ||
      StringStr (glider, "1 36") != NULL) {
    return StringSave ("SGS 1-36");
  }
  if (StringStr (glider, "Private") != NULL) {
    return StringSave ("Private");
  }

  return StringSave ("Private");
}

static Int4 SetFlightCategory (
  CharPtr category
)

{
  if (StringHasNoText (category)) return 0;

  if (StringStr (category, "Rent") != NULL) return 1;
  if (StringStr (category, "Priv") != NULL) return 1;
  if (StringStr (category, "Inst") != NULL) return 2;
  if (StringStr (category, "Demo") != NULL) return 3;
  if (StringStr (category, "Exam") != NULL) return 4;
  if (StringStr (category, "Test") != NULL) return 5;
  if (StringStr (category, "Free") != NULL) return 6;
  if (StringStr (category, "FAST") != NULL) return 7;

  return 0;
}

static QUEUE  submitquerylist = NULL;

/*
static Boolean LIBCALLBACK SubmitQueryResultProc (
  CONN conn,
  VoidPtr userdata,
  EIO_Status status
)

{
  char     buffer [64];
  size_t   n_read;
  ErrSev   oldsev;
  Boolean  res = FALSE;

  if (conn != NULL && status == eIO_Success) {
    oldsev = ErrSetMessageLevel (SEV_MAX);
    status = CONN_Read (conn, buffer, sizeof (buffer), &n_read, eIO_ReadPlain);
    if (status == eIO_Success) {
      if (StringNCmp (buffer, "okay", 4) == 0) {
        res = TRUE;
      }
    }
    ErrSetMessageLevel (oldsev);
  }
  return res;
}
*/

/*
static void SubmitUrlProc (IteM i)

{
  AsnIoConnPtr  aicp;
  BaseFormPtr   bfp;
  CONN          conn;
  LogSheetPtr   lsp;
  SscFormPtr    sfp;
  SscTablesPtr  stp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);
  if (stp == NULL) return;
  if (StringHasNoText (stp->submission_host) ||
      StringHasNoText (stp->submission_path)) return;

  lsp = (LogSheetPtr) FormToPointer (sfp->form);
  if (lsp == NULL) {
    Message (MSG_OK, "Unable to write record");
    return;
  }

  conn = QUERY_OpenUrlQuery (stp->submission_host, 80, stp->submission_path,
                             NULL, "ssclog", 30, eMIME_T_NcbiData, eMIME_AsnText,
                             eENCOD_None, 0);

  if (conn != NULL) {
    aicp = QUERY_AsnIoConnOpen ("w", conn);
    if (aicp != NULL) {
      LogSheetAsnWrite (lsp, aicp->aip, NULL);

      AsnIoFlush (aicp->aip);
      QUERY_AsnIoConnClose (aicp);

      QUERY_SendQuery (conn);

      QUERY_AddToQueue (&submitquerylist, conn, SubmitQueryResultProc, NULL, TRUE);
    } else {
      Message (MSG_OK, "Unable to open Asn connection");
    }
  } else {
    Message (MSG_OK, "Unable to open URL query");
  }

  LogSheetFree (lsp);
}
*/

static void ImportProc (IteM i)

{
  MsgAnswer        ans;
  CharPtr          array [256];
  BaseFormPtr      bfp;
  Int2             count;
  ClockTimePtr     ctp;
  FileCache        fc;
  GliderFlightPtr  fip, flights, lastfip, nextfip;
  FILE             *fp;
  Char             line [512];
  Char             path [PATH_MAX];
  SscFormPtr       sfp;
  CharPtr          str;
  SscTablesPtr     stp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  if (stp->need_to_update) {
    ans = Message (MSG_OKC, "%s", dirtymessage);
    if (ans == ANS_CANCEL) return;
  }

  ClearDirtyFlags (stp);

  flights = NULL;
  lastfip = NULL;

  if (GetInputFileName (path, sizeof (path), NULL, "TEXT")) {
    fp = FileOpen (path, "r");
    if (fp == NULL) return;
    FileCacheSetup (&fc, fp);

    str = FileCacheGetString (&fc, line, sizeof (line));
    while (str != NULL) {
      if (StringDoesHaveText (str)) {
        count = ParseALine (line, array);
        if (count >= 9) {
          fip = GliderFlightNew ();
          if (fip != NULL) {
            fip->name = SaveNameString (array [0], stp);
            AddNameToExtras (&(stp->members), fip->name);
            AddNameToExtras (&(stp->payers), fip->name);
            fip->instructor = SaveNameString (array [1], stp);
            AddNameToExtras (&(stp->instructors), fip->instructor);
            AddNameToExtras (&(stp->justcfigs), fip->instructor);
            fip->category = SetFlightCategory (array [2]);
            fip->towpilot = SaveNameString (array [3], stp);
            AddNameToExtras (&(stp->towpilots), fip->towpilot);
            fip->glider = SaveGliderString (array [4]);
            fip->takeoff = ParseClock (array [5]);
            ctp = fip->takeoff;
            if (ctp != NULL && ctp->hour > 0 && ctp->hour < 7) {
              ctp->hour += 12;
            }
            fip->landing = ParseClock (array [6]);
            ctp = fip->landing;
             if (ctp != NULL && ctp->hour > 0 && ctp->hour < 7) {
              ctp->hour += 12;
            }
            fip->flight_time = ParseTime (array [7]);
            fip->release_altitude = ParseInteger (array [8], -1);
            if (flights == NULL) {
              flights = fip;
            }
            if (lastfip != NULL) {
              lastfip->next = fip;
            }
            lastfip = fip;
          }
        }
      }

      str = FileCacheGetString (&fc, line, sizeof (line));
    }

    FileClose (fp);

    UpdateStaffControls (stp->staff);
    UpdateFlightControls (stp->glider_flights);
    UpdateFeesControls (stp->fees);
    UpdateChargesControls (stp->charges);
    UpdateAdjustmentsControls (stp->adjustments);
    UpdateBadgesControls (stp->awards);

    ClearDirtyFlags (stp);

    PointerToDialog (stp->glider_flights, flights);

    for (fip = flights; fip != NULL; fip = nextfip) {
      nextfip = fip->next;
      fip->next = NULL;
      GliderFlightFree (fip);
    }

    Update ();
  }

  ClearDirtyFlags (stp);
}

static int LIBCALLBACK SortExportList (
  VoidPtr vp1, VoidPtr vp2
)

{
  int         compare;
  Char        name1 [64], name2 [64];
  CharPtr     ptr, str1 = NULL, str2 = NULL;
  ValNodePtr  vnp1, vnp2;

  if (vp1 == NULL || vp2 == NULL) return 0;
  vnp1 = *((ValNodePtr PNTR) vp1);
  vnp2 = *((ValNodePtr PNTR) vp2);
  if (vnp1 == NULL || vnp2 == NULL) return 0;

  StringNCpy (name1, (CharPtr) vnp1->data.ptrvalue, sizeof (name1));
  StringNCpy (name2, (CharPtr) vnp2->data.ptrvalue, sizeof (name2));
  name1 [25] = '\0';
  name2 [25] = '\0';
  ptr = StringChr (name1, '*');
  if (ptr != NULL) {
    *ptr = '\0';
  }
  ptr = StringChr (name2, '*');
  if (ptr != NULL) {
    *ptr = '\0';
  }
  TrimSpacesAroundString (name1);
  TrimSpacesAroundString (name2);
  if (StringHasNoText (name1) || StringHasNoText (name2)) return 0;

  str1 = FindLastName (name1);
  str2 = FindLastName (name2);

  compare = StringICmp (str1, str2);

  if (compare > 0) {
    return 1;
  } else if (compare < 0) {
    return -1;
  }

  str1 = name1;
  str2 = name2;

  compare = StringICmp (str1, str2);

  if (compare > 0) {
    return 1;
  } else if (compare < 0) {
    return -1;
  }

  if (vnp1->choice > vnp2->choice) {
    return 1;
  } else if (vnp1->choice < vnp2->choice) {
    return -1;
  }

  return 0;
}

static void ExportProc (IteM i)

{
  BaseFormPtr      bfp;
  Char             buf [32];
  CalendarDatePtr  cdp;
  FILE             *fp;
  ValNodePtr       head, vnp;
  Char             path [PATH_MAX];
  SscFormPtr       sfp;
  SscTablesPtr     stp;
  CharPtr          str;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  sprintf (buf, "%2ld%2ldqbks.txt", (long) stp->currentDate.month,
           (long) stp->currentDate.day);
  cdp = DialogToPointer (stp->date);
  if (cdp != NULL) {
    sprintf (buf, "%2ld%2ldqbks.txt", (long) cdp->month,
             (long) cdp->day);
    cdp = CalendarDateFree (cdp);
  }
  if (buf [0] == ' ') {
    buf [0] = '0';
  }
  if (buf [2] == ' ') {
    buf [2] = '0';
  }

  if (GetOutputFileName (path, sizeof (path), buf)) {
    if (StringDoesHaveText (path)) {
      fp = FileOpen (path, "w");
      if (fp != NULL) {
        buf [0] = '\0';
        StringCpy (buf, stp->date_string);
        if (StringDoesHaveText (buf)) {
          fprintf (fp, "Skyline Soaring Charges - %s\n\n", buf);
          fprintf (fp, "Name                FAST Height    Glider       TowPlane");
          fprintf (fp, "           Time      Item           Charge\n\n");
        }
        head = NULL;
        ExportFlightsDialog (stp->glider_flights, &head);
        ExportFeesDialog (stp->fees, &head);
        ExportChargesDialog (stp->charges, &head);
        ExportAdjustmentsDialog (stp->adjustments, &head);

        head = ValNodeSort (head, SortExportList);
        for (vnp = head; vnp != NULL; vnp = vnp->next) {
          str = (CharPtr) vnp->data.ptrvalue;
          if (StringHasNoText (str)) continue;
          fprintf (fp, "%s\n", str);
        }
        head = ValNodeFreeData (head);

        fprintf (fp, "\n\nExpenses\n\n");
        head = NULL;
        ExportExpensesDialog (stp->expenses, &head);
        for (vnp = head; vnp != NULL; vnp = vnp->next) {
          str = (CharPtr) vnp->data.ptrvalue;
          if (StringHasNoText (str)) continue;
          fprintf (fp, "%s\n", str);
        }
        head = ValNodeFreeData (head);

        fprintf (fp, "\n\nName                FAST Charge     Check");
        fprintf (fp, "      Cash   Account   Voucher     Total\n\n");
        head = NULL;
        ExportPaymentsDialog (stp->payments, &head);
        for (vnp = head; vnp != NULL; vnp = vnp->next) {
          str = (CharPtr) vnp->data.ptrvalue;
          if (StringHasNoText (str)) continue;
          fprintf (fp, "%s\n", str);
        }
        head = ValNodeFreeData (head);

        FileClose (fp);
      } else {
        Message (MSG_OK, "Could not create export file");
      }
    }
  }
}

static void ArchiveProc (IteM i)

{
  BaseFormPtr      bfp;
  Char             buf [32];
  CalendarDatePtr  cdp;
  FILE             *fp;
  Char             path [PATH_MAX];
  SscFormPtr       sfp;
  StaffDataPtr     sdp;
  SscTablesPtr     stp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  sprintf (buf, "%2ld%2ldarcv.txt", (long) stp->currentDate.month,
           (long) stp->currentDate.day);
  cdp = DialogToPointer (stp->date);
  if (cdp != NULL) {
    sprintf (buf, "%2ld%2ldarcv.txt", (long) cdp->month,
             (long) cdp->day);
    cdp = CalendarDateFree (cdp);
  }
  if (buf [0] == ' ') {
    buf [0] = '0';
  }
  if (buf [2] == ' ') {
    buf [2] = '0';
  }

  if (GetOutputFileName (path, sizeof (path), buf)) {
    if (StringDoesHaveText (path)) {
      fp = FileOpen (path, "w");
      if (fp != NULL) {
        fprintf (fp, "Skyline Soaring Logsheet");
        buf [0] = '\0';
        StringCpy (buf, stp->date_string);
        if (StringDoesHaveText (buf)) {
          fprintf (fp, " - %s", buf);
        }
        sdp = DialogToPointer (stp->staff);
        if (sdp != NULL) {
          if (StringDoesHaveText (sdp->dutyofficer)) {
            fprintf (fp, " - Duty Officer: %s", sdp->dutyofficer);
          }
        }
        StaffDataFree (sdp);
        fprintf (fp, "\n\n");
        ArchiveFlightsDialog (stp->glider_flights, fp);
        ArchiveFeesDialog (stp->fees, fp);
        ArchiveChargesDialog (stp->charges, fp);
        ArchiveAdjustmentsDialog (stp->adjustments, fp);
        ArchiveExpensesDialog (stp->expenses, fp);
        ArchivePaymentsDialog (stp->payments, fp);
        ArchiveBadgesDialog (stp->awards, fp);
        ArchiveTowplaneDialog (stp->towplane_data, fp);
        FileClose (fp);
      } else {
        Message (MSG_OK, "Could not create archive file");
      }
    }
  }
}

static CharPtr memberLabel [] = {
  "",
  "regular",
  "intro",
  "visiting",
  "service",
  "guest",
  "fast",
  NULL
};

static CharPtr categoryLabel [] = {
  "",
  "regular",
  "instruction",
  "demo",
  "examination",
  "test",
  "free",
  "fast",
  NULL
};

typedef struct multifps {
  FILE  *flightsfp;
  FILE  *paysfp;
  FILE  *towsfp;
  FILE  *dutyfp;
  FILE  *membfp;
  FILE  *errsfp;
} MultiFps, PNTR MultiFpsPtr;

static void ProcessOneLogsheet (
  CharPtr filename,
  Pointer userdata
)

{
  AsnIoPtr         aip;
  Char             buf [64];
  CalendarDatePtr  cdp;
  ContactInfoPtr   cip;
  ClockTimePtr     ctp;
  DollarAmountPtr  dap;
  Char             date [64];
  Boolean          err;
  GliderFlightPtr  fip;
  FlightTimePtr    ftp;
  LogSheetPtr      lsp;
  MultiFpsPtr      mfp;
  Int2             num;
  PaymentInfoPtr   pip;
  StaffDataPtr     sdp;
  DollarAmount     tot_pay;
  TowPlaneDataPtr  tpp;

  mfp = (MultiFpsPtr) userdata;
  if (mfp == NULL) return;

  aip = AsnIoOpen (filename, "r");
  if (aip == NULL) return;
  lsp = LogSheetAsnRead (aip, NULL);
  AsnIoClose (aip);
  if (lsp == NULL) return;

  ModernizeLogSheet (lsp);

  date [0] = '\0';
  cdp = lsp->date;
  if (cdp != NULL) {
    PrintCalendarDate (cdp, date, DATE_WITH_SLASHES);
  }

  for (fip = lsp->glider_flights, num = 1; fip != NULL; fip = fip->next, num++) {
    err = FALSE;
    fprintf (mfp->flightsfp, "%s", date);
    fprintf (mfp->flightsfp, "\t");
    if (StringDoesHaveText (fip->name)) {
      fprintf (mfp->flightsfp, "%s", fip->name);
    } else {
      err = TRUE;
    }
    fprintf (mfp->flightsfp, "\t");
    if (fip->type >= 1 && fip->type <= 6) {
      fprintf (mfp->flightsfp, "%s", memberLabel [(int) fip->type]);
    }
    fprintf (mfp->flightsfp, "\t");
    if (StringDoesHaveText (fip->glider)) {
      fprintf (mfp->flightsfp, "%s", fip->glider);
    } else {
      err = TRUE;
    }
    fprintf (mfp->flightsfp, "\t");
    if (fip->type == 1 && fip->category == 0) {
      if (StringDoesHaveText (fip->name) && StringDoesHaveText (fip->instructor)) {
        /* for regular members flying with instructor, set instructional category */
        fip->category = 2;
      }
    }
    if (fip->category >= 1 && fip->category <= 7) {
      fprintf (mfp->flightsfp, "%s", categoryLabel [(int) fip->category]);
    }
    fprintf (mfp->flightsfp, "\t");
    if (StringDoesHaveText (fip->instructor)) {
      fprintf (mfp->flightsfp, "%s", fip->instructor);
    }
    fprintf (mfp->flightsfp, "\t");
    if (StringDoesHaveText (fip->towpilot)) {
      fprintf (mfp->flightsfp, "%s", fip->towpilot);
    }
    fprintf (mfp->flightsfp, "\t");
    ctp = fip->takeoff;
    if (ctp != NULL) {
      if (PrintClockTime (ctp, buf)) {
        fprintf (mfp->flightsfp, "%s", buf);
      } else {
        err = TRUE;
      }
    } else {
      err = TRUE;
    }
    fprintf (mfp->flightsfp, "\t");
    ctp = fip->landing;
    if (ctp != NULL) {
      if (PrintClockTime (ctp, buf)) {
        fprintf (mfp->flightsfp, "%s", buf);
      } else {
        err = TRUE;
      }
    } else {
      err = TRUE;
    }
    fprintf (mfp->flightsfp, "\t");
    ftp = fip->flight_time;
    if (ftp != NULL) {
      if (PrintFlightTime (ftp, buf)) {
        fprintf (mfp->flightsfp, "%s", buf);
      } else {
        err = TRUE;
      }
    } else {
      err = TRUE;
    }
    fprintf (mfp->flightsfp, "\t");
    if (fip->release_altitude >= 0) {
      fprintf (mfp->flightsfp, "%ld", (long) fip->release_altitude);
    } else {
      err = TRUE;
    }
    fprintf (mfp->flightsfp, "\t");
    dap = fip->flight_cost;
    if (dap != NULL) {
      if (PrintDollarAmount (dap, buf)) {
        fprintf (mfp->flightsfp, "%s", buf);
      } else {
        err = TRUE;
      }
    } else {
      err = TRUE;
    }
    fprintf (mfp->flightsfp, "\t");
    dap = fip->tow_cost;
    if (dap != NULL) {
      if (PrintDollarAmount (dap, buf)) {
        fprintf (mfp->flightsfp, "%s", buf);
      } else {
        err = TRUE;
      }
    } else {
      err = TRUE;
    }
    fprintf (mfp->flightsfp, "\t");
    dap = fip->total_cost;
    if (dap != NULL) {
      if (PrintDollarAmount (dap, buf)) {
        fprintf (mfp->flightsfp, "%s", buf);
      } else {
        err = TRUE;
      }
    } else {
      err = TRUE;
    }
    fprintf (mfp->flightsfp, "\n");
    if (err) {
      fprintf (mfp->errsfp, "%s - flight %2d\n", date, (int) num);
    }
  }

  for (pip = lsp->payments, num = 1; pip != NULL; pip = pip->next, num++) {
    err = FALSE;
    tot_pay.dollars = 0;
    tot_pay.cents = 0;
    fprintf (mfp->paysfp, "%s", date);
    fprintf (mfp->paysfp, "\t");
    if (StringDoesHaveText (pip->payer)) {
      fprintf (mfp->paysfp, "%s", pip->payer);
    } else {
      err = TRUE;
    }
    fprintf (mfp->paysfp, "\t");
    dap = pip->charges;
    if (dap != NULL) {
      if (PrintDollarAmount (dap, buf)) {
        fprintf (mfp->paysfp, "%s", buf);
      } else {
        err = TRUE;
      }
    } else {
      err = TRUE;
    }
    fprintf (mfp->paysfp, "\t");
    dap = pip->check_payment;
    if (dap != NULL) {
      tot_pay.dollars += dap->dollars;
      tot_pay.cents += dap->cents;
      if (tot_pay.cents >= 100) {
        tot_pay.dollars += tot_pay.cents / 100;
        tot_pay.cents = tot_pay.cents % 100;
      }
      if (PrintDollarAmount (dap, buf)) {
        fprintf (mfp->paysfp, "%s", buf);
      }
    }
    fprintf (mfp->paysfp, "\t");
    dap = pip->cash_payment;
    if (dap != NULL) {
      tot_pay.dollars += dap->dollars;
      tot_pay.cents += dap->cents;
      if (tot_pay.cents >= 100) {
        tot_pay.dollars += tot_pay.cents / 100;
        tot_pay.cents = tot_pay.cents % 100;
      }
      if (PrintDollarAmount (dap, buf)) {
        fprintf (mfp->paysfp, "%s", buf);
      }
    }
    fprintf (mfp->paysfp, "\t");
    dap = pip->account_payment;
    if (dap != NULL) {
      tot_pay.dollars += dap->dollars;
      tot_pay.cents += dap->cents;
      if (tot_pay.cents >= 100) {
        tot_pay.dollars += tot_pay.cents / 100;
        tot_pay.cents = tot_pay.cents % 100;
      }
      if (PrintDollarAmount (dap, buf)) {
        fprintf (mfp->paysfp, "%s", buf);
      }
    }
    fprintf (mfp->paysfp, "\t");
    dap = pip->voucher_payment;
    if (dap != NULL) {
      tot_pay.dollars += dap->dollars;
      tot_pay.cents += dap->cents;
      if (tot_pay.cents >= 100) {
        tot_pay.dollars += tot_pay.cents / 100;
        tot_pay.cents = tot_pay.cents % 100;
      }
      if (PrintDollarAmount (dap, buf)) {
        fprintf (mfp->paysfp, "%s", buf);
      }
    }
    fprintf (mfp->paysfp, "\t");
    dap = &tot_pay;
    if (dap != NULL) {
      if (PrintDollarAmount (dap, buf)) {
        fprintf (mfp->paysfp, "%s", buf);
      } else {
        err = TRUE;
      }
    } else {
      err = TRUE;
    }
    fprintf (mfp->paysfp, "\n");
    if (err) {
      fprintf (mfp->errsfp, "%s - payment %2d\n", date, (int) num);
    }
  }

  tpp = lsp->towplane_data;
  while (tpp != NULL) {
    fprintf (mfp->towsfp, "%s", date);
    fprintf (mfp->towsfp, "\t");
    if (StringDoesHaveText (tpp->towplane)) {
      fprintf (mfp->towsfp, "%s", tpp->towplane);
    }
    fprintf (mfp->towsfp, "\t");
    if (StringDoesHaveText (tpp->start_tach)) {
      fprintf (mfp->towsfp, "%s", tpp->start_tach);
    }
    fprintf (mfp->towsfp, "\t");
    if (StringDoesHaveText (tpp->end_tach)) {
      fprintf (mfp->towsfp, "%s", tpp->end_tach);
    }
    fprintf (mfp->towsfp, "\t");
    if (StringDoesHaveText (tpp->tach_time)) {
      fprintf (mfp->towsfp, "%s", tpp->tach_time);
    }
    fprintf (mfp->towsfp, "\t");
    if (StringDoesHaveText (tpp->gas_added)) {
      fprintf (mfp->towsfp, "%s", tpp->gas_added);
    }
    fprintf (mfp->towsfp, "\n");
    tpp = tpp->next;
  }

  sdp = lsp->staff;
  if (sdp != NULL) {
    fprintf (mfp->dutyfp, "%s", date);
    fprintf (mfp->dutyfp, "\t");
    if (StringDoesHaveText (sdp->dutyofficer)) {
      fprintf (mfp->dutyfp, "%s", sdp->dutyofficer);
    }
    fprintf (mfp->dutyfp, "\t");
    if (StringDoesHaveText (sdp->assistant)) {
      fprintf (mfp->dutyfp, "%s", sdp->assistant);
    }
    fprintf (mfp->dutyfp, "\t");
    if (StringDoesHaveText (sdp->instructor)) {
      fprintf (mfp->dutyfp, "%s", sdp->instructor);
    }
    fprintf (mfp->dutyfp, "\t");
    if (StringDoesHaveText (sdp->towpilot)) {
      fprintf (mfp->dutyfp, "%s", sdp->towpilot);
      fprintf (mfp->dutyfp, "\t");
      fprintf (mfp->dutyfp, "%s", sdp->towpilot);
    } else {
      if (StringDoesHaveText (sdp->am_towpilot)) {
        fprintf (mfp->dutyfp, "%s", sdp->am_towpilot);
      }
      fprintf (mfp->dutyfp, "\t");
      if (StringDoesHaveText (sdp->pm_towpilot)) {
        fprintf (mfp->dutyfp, "%s", sdp->pm_towpilot);
      }
    }
    fprintf (mfp->dutyfp, "\n");
  }

  for (cip = lsp->new_contacts; cip != NULL; cip = cip->next) {
    fprintf (mfp->membfp, "%s", date);
    fprintf (mfp->membfp, "\t");
    if (StringDoesHaveText (cip->name)) {
      fprintf (mfp->membfp, "%s", cip->name);
    }
    fprintf (mfp->membfp, "\n");
  }

  LogSheetFree (lsp);
}

static CharPtr annualanalysismessage =
"You will be asked to open any logsheet file in a directory.  The entire\n\
directory will be processed.  You will then be asked for a place to save\n\
the results.  This will create a tab-delimited file that can be imported\n\
into a spreadsheet or database for data analysis.";

static void AnnualAnalysisProc (IteM i)

{
  MsgAnswer    ans;
  BaseFormPtr  bfp;
  Char         ch;
  Char         buf [32];
  Char         directory [PATH_MAX];
  Boolean      eight_digits;
  Int2         j;
  MultiFps     mf;
  Char         path [PATH_MAX];
  CharPtr      ptr;
  Char         results [PATH_MAX];
  SscFormPtr   sfp;
  CharPtr      str;
  CharPtr      year = NULL;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;

  ans = Message (MSG_OKC, "%s", annualanalysismessage);
  if (ans == ANS_CANCEL) return;

  if (! GetInputFileName (directory, sizeof (directory), NULL, "TEXT")) return;

  ptr = StringRChr (directory, DIRDELIMCHR);
  if (ptr == NULL) return;
  *ptr = '\0';
  if (StringHasNoText (directory)) return;

  MemSet ((Pointer) &mf, 0, sizeof (MultiFps));

  buf [0] = '\0';
  eight_digits = TRUE;
  ptr++;
  str = ptr;
  ch = *ptr;
  for (j = 0; j < 8; j++) {
    if (! IS_DIGIT (ch)) {
      eight_digits = FALSE;
    }
    ptr++;
    ch = *ptr;
  }
  if (eight_digits) {
    if (ch == '.' || ch == '\0') {
      *ptr = '\0';
      year = str + 4;
    }
  }
  if (year == NULL) {
    year = "0000";
  }

  StringCpy (buf, year);
  StringCat (buf, ".sum");

  if (! GetOutputFileName (path, sizeof (path), buf)) return;
  ptr = StringRChr (path, DIRDELIMCHR);
  if (ptr == NULL) return;
  *ptr = '\0';
  if (StringHasNoText (path)) return;

  StringCpy (results, path);
  sprintf (buf, "%sflts.sum", year);
  FileBuildPath (results, NULL, buf);
  mf.flightsfp = FileOpen (results, "w");

  StringCpy (results, path);
  sprintf (buf, "%spays.sum", year);
  FileBuildPath (results, NULL, buf);
  mf.paysfp = FileOpen (results, "w");

  StringCpy (results, path);
  sprintf (buf, "%stows.sum", year);
  FileBuildPath (results, NULL, buf);
  mf.towsfp = FileOpen (results, "w");

  StringCpy (results, path);
  sprintf (buf, "%sduty.sum", year);
  FileBuildPath (results, NULL, buf);
  mf.dutyfp = FileOpen (results, "w");

  StringCpy (results, path);
  sprintf (buf, "%smemb.sum", year);
  FileBuildPath (results, NULL, buf);
  mf.membfp = FileOpen (results, "w");

  StringCpy (results, path);
  sprintf (buf, "%serrs.sum", year);
  FileBuildPath (results, NULL, buf);
  mf.errsfp = FileOpen (results, "w");

  if (mf.flightsfp != NULL &&
      mf.paysfp != NULL &&
      mf.towsfp != NULL &&
      mf.dutyfp != NULL &&
      mf.membfp != NULL &&
      mf.errsfp != NULL) {
    DirExplore (directory, NULL, ".txt", FALSE, ProcessOneLogsheet, (Pointer) &mf);
  }

  FileClose (mf.errsfp);
  FileClose (mf.membfp);
  FileClose (mf.dutyfp);
  FileClose (mf.towsfp);
  FileClose (mf.paysfp);
  FileClose (mf.flightsfp);
}

static CharPtr dailyanalysismessage =
"You will be asked to open a logsheet file in a directory.  You will\n\
then be asked for a place to save the results.  This will create a tab-\n\
delimited file that can be imported into a spreadsheet or database\n\
for data analysis.";

static void DailyAnalysisProc (IteM i)

{
  MsgAnswer    ans;
  BaseFormPtr  bfp;
  Char         ch;
  Char         buf [32];
  Char         directory [PATH_MAX];
  Boolean      eight_digits;
  Int2         j;
  MultiFps     mf;
  CharPtr      monthday = NULL;
  Char         path [PATH_MAX];
  CharPtr      ptr;
  Char         results [PATH_MAX];
  SscFormPtr   sfp;
  CharPtr      str;
  Char         tmp [32];

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;

  ans = Message (MSG_OKC, "%s", dailyanalysismessage);
  if (ans == ANS_CANCEL) return;

  if (! GetInputFileName (directory, sizeof (directory), NULL, "TEXT")) return;

  ptr = StringRChr (directory, DIRDELIMCHR);
  if (ptr == NULL) return;
  if (StringHasNoText (directory)) return;

  MemSet ((Pointer) &mf, 0, sizeof (MultiFps));

  buf [0] = '\0';
  eight_digits = TRUE;
  ptr++;
  str = ptr;
  ch = *ptr;
  for (j = 0; j < 8; j++) {
    if (! IS_DIGIT (ch)) {
      eight_digits = FALSE;
    }
    ptr++;
    ch = *ptr;
  }
  if (eight_digits) {
    if (ch == '.' || ch == '\0') {
      StringCpy (tmp, str);
      tmp [4] = '\0';
      monthday = tmp;
    }
  }
  if (monthday == NULL) {
    monthday = "0000";
  }

  StringCpy (buf, monthday);
  StringCat (buf, ".sum");

  if (! GetOutputFileName (path, sizeof (path), buf)) return;
  ptr = StringRChr (path, DIRDELIMCHR);
  if (ptr == NULL) return;
  *ptr = '\0';
  if (StringHasNoText (path)) return;

  StringCpy (results, path);
  sprintf (buf, "%sflts.sum", monthday);
  FileBuildPath (results, NULL, buf);
  mf.flightsfp = FileOpen (results, "w");

  StringCpy (results, path);
  sprintf (buf, "%spays.sum", monthday);
  FileBuildPath (results, NULL, buf);
  mf.paysfp = FileOpen (results, "w");

  StringCpy (results, path);
  sprintf (buf, "%stows.sum", monthday);
  FileBuildPath (results, NULL, buf);
  mf.towsfp = FileOpen (results, "w");

  StringCpy (results, path);
  sprintf (buf, "%sduty.sum", monthday);
  FileBuildPath (results, NULL, buf);
  mf.dutyfp = FileOpen (results, "w");

  StringCpy (results, path);
  sprintf (buf, "%smemb.sum", monthday);
  FileBuildPath (results, NULL, buf);
  mf.membfp = FileOpen (results, "w");

  StringCpy (results, path);
  sprintf (buf, "%serrs.sum", monthday);
  FileBuildPath (results, NULL, buf);
  mf.errsfp = FileOpen (results, "w");

  if (mf.flightsfp != NULL &&
      mf.paysfp != NULL &&
      mf.towsfp != NULL &&
      mf.dutyfp != NULL &&
      mf.membfp != NULL &&
      mf.errsfp != NULL) {
    ProcessOneLogsheet (directory, (Pointer) &mf);
  }

  FileClose (mf.errsfp);
  FileClose (mf.membfp);
  FileClose (mf.dutyfp);
  FileClose (mf.towsfp);
  FileClose (mf.paysfp);
  FileClose (mf.flightsfp);
}

#ifdef WIN_MAC
static void SSCFormActivate (WindoW w)

{
  currentFormDataPtr = (VoidPtr) GetObjectExtra (w);
}

static void MacDeactProc (WindoW w)

{
  currentFormDataPtr = NULL;
}
#endif

static void SscFormMessage (ForM f, Int2 mssg)

{
  BaseFormPtr  bfp;

  bfp = (BaseFormPtr) GetObjectExtra (f);
  if (bfp == NULL) return;

  switch (mssg) {
    case VIB_MSG_CUT :
      StdCutTextProc (NULL);
      break;
    case VIB_MSG_COPY :
      StdCopyTextProc (NULL);
      break;
    case VIB_MSG_PASTE :
      StdPasteTextProc (NULL);
      break;
    case VIB_MSG_DELETE :
      StdDeleteTextProc (NULL);
      break;
    default :
      break;
  }
}

static Boolean DisplayValidatorMessage (
  Int4       validator_result,
  Boolean    ask_to_cancel,
  Boolean    announce_ok,
  SscFormPtr sfp
)

{
  MsgAnswer   ans;
  Char        buf [1200];
  Char        buf_warn[200];
  LogSheetPtr lsp;

  buf [0] = '\0';
  buf_warn [0] = '\0';
  if ((validator_result & INSTRUCTING_SELF) != 0) {
    StringCat (buf, "Glider pilot and instructor are identical.\n");
  }
  if ((validator_result & TOWING_SELF) != 0) {
    StringCat (buf, "Glider pilot and tow pilot are identical.\n");
  }
  if ((validator_result & INSTRUCTOR_TOWS) != 0) {
    StringCat (buf, "Instructor and tow pilot are identical.\n");
  }
  if ((validator_result & INSTRUCTOR_MISSING) != 0) {
    StringCat (buf, "Instructor missing from instructional flight.\n");
  }
  if ((validator_result & INCONSISTENT_CLASS) != 0) {
    StringCat (buf, "Instructor should not be set for normal flight.\n");
  }
  if ((validator_result & TOW_PILOT_MISSING) != 0) {
    StringCat (buf, "Tow pilot missing from a flight.\n");
  }
  if ((validator_result & GLIDER_MISSING) != 0) {
    StringCat (buf, "Glider missing on a flight.\n");
  }
  if ((validator_result & RELEASE_HEIGHT_MISSING) != 0) {
    StringCat (buf, "Release height missing on a flight.\n");
  }
  if ((validator_result & TAKEOFF_TIME_MISSING) != 0) {
    StringCat (buf, "Takeoff time missing on a flight.\n");
  }
  if ((validator_result & LANDING_TIME_MISSING) != 0) {
    StringCat (buf, "Landing time missing on a flight.\n");
  }
  if ((validator_result & PAYMENT_PROBLEM) != 0) {
    StringCat (buf, "Payment does not match charge.\n");
  }
  if ((validator_result & HOME_PHONE_MISSING) != 0) {
    StringCat (buf, "Home phone missing on a contact.\n");
  }
  if ((validator_result & E_MAIL_MISSING) != 0) {
    StringCat (buf, "E-mail missing on a contact.\n");
  }
  if ((validator_result & BAD_TACH_TIME) != 0) {
    StringCat (buf, "Tach time has problems. Did you miss a digit before or after the decimal point?\n");
  }
  if ((validator_result & BAD_GAS_ADDED) != 0) {
    StringCat (buf, "Gas added has problems. Did you miss a digit before or after the decimal point?\n");
  }
  //
  if ((validator_result & AIRPORT_MISSING) != 0) {
    StringCat (buf, "Airport not specified.\n");
  } else {
    // Check if the specified airport is not SSCs home airport
     if (sfp != NULL) {
        lsp = (LogSheetPtr) FormToPointer (sfp->form);

        if (lsp != NULL) {
           if (StringICmp (lsp->airport, (&(sfp->tables))->field_list->airport) != 0) {

              sprintf(buf_warn, "Warning: The selected airport %s is different from club's home airport %s.\n",
                                 lsp->airport,
                                 (&(sfp->tables))->field_list->airport);
              //StringCat (buf_warn, "Please check if this is correct.\n");
           }
           LogSheetFree (lsp);
        }
     }
  }

  if (StringHasNoText (buf) && StringHasNoText (buf_warn)) {
    if (announce_ok) {
      Message (MSG_OK, "%s", "Validation succeeded\n");
    }
    return TRUE;
  }

  StringCat (buf, buf_warn);

  if (ask_to_cancel) {
    ans = Message (MSG_OKC, "%sDo you wish to continue quitting?", buf);
    if (ans == ANS_CANCEL) return FALSE;
  } else {
    Message (MSG_OK, "%s", buf);
    return TRUE;
  }
  return TRUE;
}

static void ValidateProc (
  IteM i
)

{
  BaseFormPtr  bfp;
  Int4         result = 0;
  SscFormPtr   sfp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;

  result = ValidateLogsheet (sfp, FALSE);

  DisplayValidatorMessage (result, FALSE, TRUE, sfp);
}

static CharPtr quitmessage = "Do you wish to quit without saving changes?";

static void QuitProc (
  IteM i
)

{
  MsgAnswer     ans;
  BaseFormPtr   bfp;
  Int4          result = 0;
  SscFormPtr    sfp;
  SscTablesPtr  stp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (i);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  if (stp->need_to_update) {
    ans = Message (MSG_OKC, "%s", quitmessage);
    if (ans == ANS_CANCEL) return;
  }

  result = ValidateLogsheet (sfp, TRUE);

  if (! DisplayValidatorMessage (result, TRUE, FALSE, sfp)) return;

  QuitProgram ();
}

static void CloseProc (
  WindoW w
)

{
  /*
  MsgAnswer     ans;
  BaseFormPtr   bfp;
  SscFormPtr    sfp;
  SscTablesPtr  stp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (w);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  if (stp->need_to_update) {
    ans = Message (MSG_OKC, "%s", quitmessage);
    if (ans == ANS_CANCEL) return;
  }

  QuitProgram ();
  */
}

static void AutoSaveProc (
  ChoicE c
)

{
  BaseFormPtr   bfp;
  SscFormPtr    sfp;
  SscTablesPtr  stp;

#ifdef WIN_MAC
  bfp = currentFormDataPtr;
#else
  bfp = GetObjectExtra (c);
#endif
  sfp = (SscFormPtr) bfp;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  stp->allow_auto_save = (Boolean) (GetStatus (c) == 1);
}

static CharPtr about_prefix = "Skyline Soaring Club Logsheet version ";
static CharPtr about_suffix = "\nCopyright (c) 2004-2019. All rights reserved.";

static void AboutProc (
  IteM i
)

{
  Message (MSG_OK, "%s%s%s", about_prefix, program_version, about_suffix);
}

#ifdef WIN_MAC
static void SetupMacMenus (
  Boolean allow_url
)

{
  ChoicE  c;
  /*
  IteM    i;
  */
  MenU    m;
  MenU    s;

  m = AppleMenu (NULL);
  CommandItem (m, "About...", AboutProc);
  SeparatorItem (m);
  DeskAccGroup (m);

  m = PulldownMenu (NULL, "File");
  CommandItem (m, "Open.../O", OpenLogProc);
  SeparatorItem (m);
  CommandItem (m, "Save to Hard Disk/S", SaveLogProc);
  CommandItem (m, "Save to Floppy", SaveFloppyLogProc);
  CommandItem (m, "Save to Flash/F", SaveFlashLogProc);
  SeparatorItem (m);
  CommandItem (m, "Save As...", SaveAsLogProc);
  SeparatorItem (m);
  /*
  i = CommandItem (m, "Transmit Logsheet", SubmitUrlProc);
  if (! allow_url) {
    Disable (i);
  }
  SeparatorItem (m);
  */
  s = SubMenu (m, "Automatically Save Backup");
  c = ChoiceGroup (s, AutoSaveProc);
  ChoiceItem (c, "On");
  ChoiceItem (c, "Off");
  SetValue (c, 2);
  SeparatorItem (m);
  CommandItem (m, "Import Spreadsheet", ImportProc);
  CommandItem (m, "Export for QuickBooks", ExportProc);
  CommandItem (m, "Archive for Printing", ArchiveProc);
  CommandItem (m, "Convert to XML", SaveAsXMLProc);
  SeparatorItem (m);
  CommandItem (m, "Validate Logsheet", ValidateProc);
  SeparatorItem (m);
  s = SubMenu (m, "Data Analysis");
  CommandItem (s, "Annual", AnnualAnalysisProc);
  CommandItem (s, "Daily", DailyAnalysisProc);
  SeparatorItem (m);
  CommandItem (m, "Quit/Q", QuitProc);
}
#endif

#ifndef WIN_MAC
static void SetupPCMenus (
  WindoW w,
  SscFormPtr sfp,
  Boolean allow_url
)

{
  ChoicE  c;
  IteM    i;
  MenU    m;
  MenU    s;

  m = PulldownMenu (w, "File");
  CommandItem (m, "About...", AboutProc);
  SeparatorItem (m);
  i = CommandItem (m, "Open.../  O", OpenLogProc);
  SetObjectExtra (i, sfp, NULL);
  SeparatorItem (m);
  i = CommandItem (m, "Save to Hard Disk/  S", SaveLogProc);
  SetObjectExtra (i, sfp, NULL);
  i = CommandItem (m, "Save to Floppy", SaveFloppyLogProc);
  SetObjectExtra (i, sfp, NULL);
  i = CommandItem (m, "Save to Flash/  F", SaveFlashLogProc);
  SetObjectExtra (i, sfp, NULL);
  SeparatorItem (m);
  i = CommandItem (m, "Save As...", SaveAsLogProc);
  SetObjectExtra (i, sfp, NULL);
  SeparatorItem (m);
  /*
  i = CommandItem (m, "Transmit Logsheet", SubmitUrlProc);
  SetObjectExtra (i, sfp, NULL);
  if (! allow_url) {
    Disable (i);
  }
  SeparatorItem (m);
  */
  s = SubMenu (m, "Automatically Save Backup");
  c = ChoiceGroup (s, AutoSaveProc);
  SetObjectExtra (c, sfp, NULL);
  ChoiceItem (c, "On");
  ChoiceItem (c, "Off");
  SetValue (c, 1);
  SeparatorItem (m);
  i = CommandItem (m, "Import Spreadsheet", ImportProc);
  SetObjectExtra (i, sfp, NULL);
  i = CommandItem (m, "Export for QuickBooks", ExportProc);
  SetObjectExtra (i, sfp, NULL);
  i = CommandItem (m, "Archive for Printing", ArchiveProc);
  SetObjectExtra (i, sfp, NULL);
  i = CommandItem (m, "Convert to XML", SaveAsXMLProc);
  SetObjectExtra (i, sfp, NULL);
  SeparatorItem (m);
  i = CommandItem (m, "Validate Logsheet", ValidateProc);
  SetObjectExtra (i, sfp, NULL);
  SeparatorItem (m);
  s = SubMenu (m, "Data Analysis");
  i = CommandItem (s, "Annual", AnnualAnalysisProc);
  SetObjectExtra (i, sfp, NULL);
  i = CommandItem (s, "Daily", DailyAnalysisProc);
  SetObjectExtra (i, sfp, NULL);
  SeparatorItem (m);
  i = CommandItem (m, "Quit/  Q", QuitProc);
  SetObjectExtra (i, sfp, NULL);
}
#endif

extern SscTablesPtr GetTablesPointer (
  BaseFormPtr bfp
)

{
  SscFormPtr  sfp;

  if (bfp == NULL) return NULL;
  sfp = (SscFormPtr) bfp;
  return &(sfp->tables);
}

static BaseFormPtr globalSscForm = NULL;

static void HandleArrowKeys (
  Char key
)

{
  SscFormPtr    sfp;
  SubPagePtr    spp;
  SscTablesPtr  stp;

  sfp = (SscFormPtr) globalSscForm;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  if (key != NLM_UP && key != NLM_DOWN) return;

  spp = &(sfp->mainPage);

  switch (spp->currPage) {
    case STARTUP_PAGE :
      break;
    case PERSONNEL_PAGE :
      spp = &(sfp->personnelPage);
      break;
    case OPERATIONS_PAGE :
      spp = &(sfp->operationsPage);
      switch (spp->currPage) {
        case GLIDER_FLIGHTS_PAGE :
          KeyToFlightsDialog (stp->glider_flights, key);
          break;
        default :
          break;
      }
      break;
    case FINANCES_PAGE :
      spp = &(sfp->financesPage);
      switch (spp->currPage) {
        case PAYMENTS_PAGE :
          KeyToPaymentsDialog (stp->payments, key);
          break;
        case FEES_PAGE :
          KeyToFeesDialog (stp->fees, key);
          break;
        case CHARGES_PAGE :
          KeyToChargesDialog (stp->charges, key);
          break;
        case ADJUSTMENTS_PAGE :
          KeyToAdjustmentsDialog (stp->adjustments, key);
          break;
        default :
          break;
      }
      break;
    case SHUTDOWN_PAGE :
      spp = &(sfp->shutdownPage);
      switch (spp->currPage) {
        case CONTACTS_PAGE :
          KeyToContactsDialog (stp->new_contacts, key);
          break;
        case BADGE_PAGE :
          KeyToBadgesDialog (stp->awards, key);
          break;
        default :
          break;
      }
      break;
    default :
      break;
  }
}

static void HandleTimer (
  WindoW w
)

{
  ClockTime     currTime;
  DayTime       dt;
  SscFormPtr    sfp;
  SscTablesPtr  stp;

  sfp = (SscFormPtr) globalSscForm;
  if (sfp == NULL) return;
  stp = &(sfp->tables);

  QUERY_CheckQueue (&submitquerylist);

  if (stp->auto_delay > 0) {
    (stp->auto_delay)--;
  }

  if (GetDayTime (&dt)) {
    if (dt.tm_hour == 0) {
      dt.tm_hour = 24;
    }
    currTime.hour = dt.tm_hour;
    currTime.minute = dt.tm_min;

    if (currTime.hour == stp->currentTime.hour &&
        currTime.minute == stp->currentTime.minute) return;

    stp->currentTime.hour = currTime.hour;
    stp->currentTime.minute = currTime.minute;
  }

  FlightControlsTimerProc (stp->glider_flights);

  if (stp->need_to_auto_save && stp->auto_delay == 0) {
    stp->need_to_auto_save = FALSE;
    if (stp->allow_auto_save) {
      SaveAutoProc (sfp);
    }
  }
}

typedef struct dstdate {
  Int2  month;
  Int2  day;
} DstDate, PNTR DstDatePtr;

static DstDate dst_start [] = {
  { 4,  2}, /* 2000 */
  { 4,  1}, /* 2001 */
  { 4,  7}, /* 2002 */
  { 4,  6}, /* 2003 */
  { 4,  4}, /* 2004 */
  { 4,  3}, /* 2005 */
  { 4,  2}, /* 2006 */
  { 3, 11}, /* 2007 */
  { 3,  9}, /* 2008 */
  { 3,  8}, /* 2009 */
  { 3, 14}, /* 2010 */
  { 3, 13}, /* 2011 */
  { 3, 11}, /* 2012 */
  { 3, 10}, /* 2013 */
  { 3,  9}, /* 2014 */
  { 3,  8}, /* 2015 */
  { 3, 13}, /* 2016 */
  { 3, 12}, /* 2017 */
  { 3, 11}, /* 2018 */
  { 3, 10}, /* 2019 */
  { 3,  8}, /* 2020 */
  { 3,  14}, /* 2021 */
  { 3,  13}, /* 2022 */
  { 3,  12}, /* 2023 */
  { 3,  10}, /* 2024 */
  { 3,   9}, /* 2025 */
  { 0,  0}
};

static DstDate dst_stop [] = {
  {10, 29}, /* 2000 */
  {10, 28}, /* 2001 */
  {10, 27}, /* 2002 */
  {10, 26}, /* 2003 */
  {10, 31}, /* 2004 */
  {10, 30}, /* 2005 */
  {10, 29}, /* 2006 */
  {11,  4}, /* 2007 */
  {11,  2}, /* 2008 */
  {11,  1}, /* 2009 */
  {11,  7}, /* 2010 */
  {11,  6}, /* 2011 */
  {11,  4}, /* 2012 */
  {11,  3}, /* 2013 */
  {11,  2}, /* 2014 */
  {11,  1}, /* 2015 */
  {11,  6}, /* 2016 */
  {11,  5}, /* 2017 */
  {11,  4}, /* 2018 */
  {11,  3}, /* 2019 */
  {11,  1}, /* 2020 */
  {11,  7}, /* 2021 */
  {11,  6}, /* 2022 */
  {11,  5}, /* 2023 */
  {11,  3}, /* 2024 */
  {11,  2}, /* 2025 */
  { 0,  0}
};

Int2 Main (
  void
)

{
  Boolean        allow_url;
  Char           buf [64];
  DayTime        dt;
  GrouP          g, h;
  ValNodePtr     head = NULL;
  MenU           m;
  Char           path [PATH_MAX];
  RecT           r;
  SscFormPtr     sfp;
  SubPagePtr     spp;
  CharPtr        str;
  SscTablesPtr   stp;
  DialoG         tbs;
  MemberInfoPtr  tmp;
  Char           ttl [128];
  ValNodePtr     vnp;
  WindoW         w;
  Int2           wid;
  Int2           yr;

  ErrSetFatalLevel (SEV_MAX);
  ErrClearOptFlags (EO_SHOW_USERSTR);
  ErrPathReset ();

  sfp = (SscFormPtr) MemNew (sizeof (SscForm));
  if (sfp == NULL) return 1;
  stp = &(sfp->tables);

  if (GetDayTime (&dt)) {
    stp->workingDate.month = dt.tm_mon + 1;
    stp->workingDate.day = dt.tm_mday;
    stp->workingDate.year = dt.tm_year + 1900;

    stp->currentDate.month = dt.tm_mon + 1;
    stp->currentDate.day = dt.tm_mday;
    stp->currentDate.year = dt.tm_year + 1900;

    if (stp->currentDate.year >= 2000 && stp->currentDate.year <= 2025) {
      yr = (Int2) (stp->currentDate.year - 2000);
      if (stp->currentDate.month > dst_start [yr].month && stp->currentDate.month < dst_stop [yr].month) {
        stp->is_dst = TRUE;
      } else if (stp->currentDate.month == dst_start [yr].month &&
                 stp->currentDate.day >= dst_start [yr].day) {
        stp->is_dst = TRUE;
      } else if (stp->currentDate.month == dst_stop [yr].month &&
                 stp->currentDate.day < dst_stop [yr].day) {
        stp->is_dst = TRUE;
      }
    }

    if (dt.tm_hour == 0) {
      dt.tm_hour = 24;
    }
    stp->currentTime.hour = dt.tm_hour;
    stp->currentTime.minute = dt.tm_min;
  }

  PrintCalendarDate (&(stp->currentDate), buf, DATE_WITH_DAY);
  StringNCpy (stp->date_string, buf, sizeof (stp->date_string));
  sprintf (ttl, "Skyline Soaring Logsheet %s - %s", program_version, buf);

  ProgramPath (path, sizeof (path));
  if (! ReadMemberTable ("members.txt", &(sfp->tables))) {
    Message (MSG_FATAL, "Unable to open members.txt file, program located at %s", path);
    return 1;
  }
  if (! ReadAirportTable ("airports.txt", &(sfp->tables))) {
    Message (MSG_FATAL, "Unable to open airports.txt file, program located at %s", path);
    return 1;
  }
  if (! ReadGliderTable ("gliders.txt", &(sfp->tables))) {
    Message (MSG_FATAL, "Unable to open gliders.txt file, program located at %s", path);
    return 1;
  }
  if (! ReadTowRateTable ("towrate.txt", &(sfp->tables))) {
    Message (MSG_FATAL, "Unable to open towrate.txt file, program located at %s", path);
    return 1;
  }
  if (! ReadMiscRateTable ("miscrate.txt", &(sfp->tables))) {
    Message (MSG_FATAL, "Unable to open miscrate.txt file, program located at %s", path);
    return 1;
  }
  ReadFieldTable ("fields.txt", &(sfp->tables));
  if (ReadTempMemberTable ("tempmemb.txt", &(sfp->tables), &(stp->currentDate))) {
    for (tmp = stp->intro_member_table; tmp != NULL; tmp = tmp->next) {
      str = (CharPtr) tmp->name;
      if (StringHasNoText (str)) continue;
      AddNameToExtras (&(stp->members), str);
      AddNameToExtras (&(stp->payers), str);
    }
    RecreateNameAlist (&(stp->members));
    RecreateNameAlist (&(stp->payers));
  }
  head = ReadPayersTable ("payers.txt");
  if (head != NULL) {
    for (vnp = head; vnp != NULL; vnp = vnp->next) {
      str = (CharPtr) vnp->data.ptrvalue;
      if (StringHasNoText (str)) continue;
      AddNameToExtras (&(stp->payers), str);
    }
    RecreateNameAlist (&(stp->payers));
    ValNodeFreeData (head);
  }
  if (StringDoesHaveText (stp->club)) {
    sprintf (ttl, "%s Logsheet %s - %s", stp->club, program_version, buf);
  }

  ClearDirtyFlags (stp);

  allow_url = (Boolean) (stp->submission_host != NULL && stp->submission_path != NULL);

#ifdef WIN_MAC
  SetDeactivate (NULL, MacDeactProc);
  SetupMacMenus (allow_url);
#endif

  w = FixedWindow (-50, -33, -10, -10, ttl, CloseProc);
#ifndef WIN_MAC
  SetupPCMenus (w, sfp, allow_url);
#endif

  stp->allow_auto_save = TRUE;

  SetObjectExtra (w, sfp, StdCleanupFormProc);
  sfp->form = (ForM) w;
  sfp->activate = NULL;
  sfp->fromform = SscFormToLogSheetPtr;
  sfp->formmessage = SscFormMessage;

#ifdef WIN_MAC
  SetActivate (w, SSCFormActivate);
#endif

  spp = &(sfp->mainPage);

  tbs = CreateFolderTabs (w, mainTitles, 0, 0, 0, SYS_FOLDER_TAB,
                          ChangeMainTabs, (Pointer) sfp);

  ObjectRect (tbs, &r);
  wid = r.right - r.left;

  h = HiddenGroup (w, 0, 0, NULL);

  g = HiddenGroup (h, -1, 0, NULL);
  CreateStartupGroup (g, sfp, wid);
  spp->pages [STARTUP_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  CreatePersonnelGroup (g, sfp);
  Hide (g);
  spp->pages [PERSONNEL_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  CreateOperationsGroup (g, sfp);
  Hide (g);
  spp->pages [OPERATIONS_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  CreateFinancesGroup (g, sfp);
  Hide (g);
  spp->pages [FINANCES_PAGE] = g;

  g = HiddenGroup (h, -1, 0, NULL);
  CreateShutdownGroup (g, sfp);
  Hide (g);
  spp->pages [SHUTDOWN_PAGE] = g;

  spp->currPage = 0;

  Show (spp->pages [spp->currPage]);

  AlignObjects (ALIGN_CENTER, (HANDLE) tbs,
                (HANDLE) spp->pages [STARTUP_PAGE],
                (HANDLE) spp->pages [PERSONNEL_PAGE],
                (HANDLE) spp->pages [OPERATIONS_PAGE],
                (HANDLE) spp->pages [FINANCES_PAGE],
                (HANDLE) spp->pages [SHUTDOWN_PAGE],
                NULL);

  RealizeWindow (w);

#ifdef WIN_MAC
  m = PulldownMenu (NULL, "Edit");
  cutItem = FormCommandItem (m, CUT_MENU_ITEM, NULL, VIB_MSG_CUT);
  copyItem = FormCommandItem (m, COPY_MENU_ITEM, NULL, VIB_MSG_COPY);
  pasteItem = FormCommandItem (m, PASTE_MENU_ITEM, NULL, VIB_MSG_PASTE);
  deleteItem = FormCommandItem (m, CLEAR_MENU_ITEM, NULL, VIB_MSG_DELETE);
  sfp->editMenu = m;

  m = PulldownMenu (NULL, "Flight");
  SetupFlightMenus (m, NULL);
  sfp->flightMenu = m;

  m = PulldownMenu (NULL, "Payment");
  SetupPaymentMenus (m, NULL);
  sfp->paymentMenu = m;
#else
  m = PulldownMenu (w, "Edit");
  FormCommandItem (m, CUT_MENU_ITEM, (BaseFormPtr) sfp, VIB_MSG_CUT);
  FormCommandItem (m, COPY_MENU_ITEM, (BaseFormPtr) sfp, VIB_MSG_COPY);
  FormCommandItem (m, PASTE_MENU_ITEM, (BaseFormPtr) sfp, VIB_MSG_PASTE);
  FormCommandItem (m, CLEAR_MENU_ITEM, (BaseFormPtr) sfp, VIB_MSG_DELETE);
  sfp->editMenu = m;

  m = PulldownMenu (w, "Flight");
  SetupFlightMenus (m, (BaseFormPtr) sfp);
  sfp->flightMenu = m;

  m = PulldownMenu (w, "Payment");
  SetupPaymentMenus (m, (BaseFormPtr) sfp);
  sfp->paymentMenu = m;
#endif

  Disable (sfp->flightMenu);
  Disable (sfp->paymentMenu);
  Disable (sfp->personMenu);

  globalSscForm = (BaseFormPtr) sfp;

  KeyboardView (HandleArrowKeys);
  SetWindowTimer (w, HandleTimer);

  PopulateHelpDoc (sfp->helpDoc, "sschelp.txt");

  CheckForExistingLogFile (sfp);

  Show (w);
  ProcessEvents ();
  return 0;
}

