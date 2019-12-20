/*
    SKYLINE SOARING CLUB DUTY LOGSHEET PROGRAM - Date page

    Copyright (c) 2004-2011 Jonathan A. Kans. All rights reserved.

    Change Log:

    Ver   Date       Author           Comment
    ----- ---------- ---------------- ------------------------------------------
    1.0              Jonathan Kans    Created
    1.1   01/03/2016 Ertan Tete       Added years 2016-2019 to enum years_alist
    1.2   12/20/2019 Chris Norris     Added 2020
*/

#include "ssclog.h"

/* dialog controls */

typedef struct datepage {
  DIALOG_MESSAGE_BLOCK
  PopuP  month;
  PopuP  day;
  PopuP  year;
} DatePage, PNTR DatePagePtr;

static ENUM_ALIST(months_alist)
  {" ",    -1},
  {"Jan",   1},
  {"Feb",   2},
  {"Mar",   3},
  {"Apr",   4},
  {"May",   5},
  {"Jun",   6},
  {"Jul",   7},
  {"Aug",   8},
  {"Sep",   9},
  {"Oct",  10},
  {"Nov",  11},
  {"Dec",  12},
END_ENUM_ALIST

static ENUM_ALIST(days_alist)
  {" ",   -1},
  {" 1",   1},
  {" 2",   2},
  {" 3",   3},
  {" 4",   4},
  {" 5",   5},
  {" 6",   6},
  {" 7",   7},
  {" 8",   8},
  {" 9",   9},
  {"10",  10},
  {"11",  11},
  {"12",  12},
  {"13",  13},
  {"14",  14},
  {"15",  15},
  {"16",  16},
  {"17",  17},
  {"18",  18},
  {"19",  19},
  {"20",  20},
  {"21",  21},
  {"22",  22},
  {"23",  23},
  {"24",  24},
  {"25",  25},
  {"26",  26},
  {"27",  27},
  {"28",  28},
  {"29",  29},
  {"30",  30},
  {"31",  31},
END_ENUM_ALIST

static ENUM_ALIST(years_alist)
  {" ",       -1},
  {"2004",  2004},
  {"2005",  2005},
  {"2006",  2006},
  {"2007",  2007},
  {"2008",  2008},
  {"2009",  2009},
  {"2010",  2010},
  {"2011",  2011},
  {"2012",  2012},
  {"2013",  2013},
  {"2014",  2014},
  {"2015",  2015},
  {"2016",  2016},
  {"2017",  2017},
  {"2018",  2018},
  {"2019",  2019},
  {"2020",  2020},
END_ENUM_ALIST

static void ChangeDateProc (
  PopuP p
)

{
  DialogActnFunc  actproc;
  DatePagePtr     dpp;

  dpp = (DatePagePtr) GetObjectExtra (p);
  if (dpp == NULL) return;

  actproc = dpp->actproc;
  if (actproc != NULL) {
    actproc (dpp->dialog);
  }
}

static void CalendarDatePtrToDatePage (
  DialoG d,
  Pointer data
)

{
  CalendarDatePtr  cdp;
  DatePagePtr      dpp;

  dpp = (DatePagePtr) GetObjectExtra (d);
  if (dpp == NULL) return;
  cdp = (CalendarDatePtr) data;
  if (cdp != NULL) {
    SetEnumPopup (dpp->month, months_alist, (UIEnum) cdp->month);
    SetEnumPopup (dpp->day, days_alist, (UIEnum) cdp->day);
    SetEnumPopup (dpp->year, years_alist, (UIEnum) cdp->year);
    return;
  }

  SafeSetValue (dpp->month, 1);
  SafeSetValue (dpp->day, 1);
  SafeSetValue (dpp->year, 1);
}

static Pointer DatePageToCalendarDatePtr (
  DialoG d
)

{
  CalendarDatePtr  cdp;
  UIEnum           day;
  DatePagePtr      dpp;
  UIEnum           month;
  UIEnum           year;

  cdp = NULL;
  dpp = (DatePagePtr) GetObjectExtra (d);
  if (dpp == NULL) return NULL;
  cdp = CalendarDateNew ();
  if (cdp != NULL) {

    if (GetEnumPopup (dpp->month, months_alist, &month)) {
      cdp->month = (Int4) month;
    } else {
      cdp->month = -1;
    }

    if (GetEnumPopup (dpp->day, days_alist, &day)) {
      cdp->day = (Int4) day;
    } else {
      cdp->day = -1;
    }

    if (GetEnumPopup (dpp->year, years_alist, &year)) {
      cdp->year = (Int4) year;
    } else {
      cdp->year = -1;
    }

    /*
    if (cdp->month == -1 || cdp->day == -1 || cdp->year == -1) {
      cdp = CalendarDateFree (cdp);
    }
    */
  }
  return (Pointer) cdp;
}

extern void GetCalendarDate (
  DialoG d,
  Int4Ptr month,
  Int4Ptr day,
  Int4Ptr year
)

{
  CalendarDatePtr  cdp;

  if (d == NULL || month == NULL || day == NULL || year == NULL) return;

  cdp = (CalendarDatePtr) DialogToPointer (d);
  if (cdp != NULL) {
    *month = cdp->month;
    *day = cdp->day;
    *year = cdp->year;
    cdp = CalendarDateFree (cdp);
    return;
  }

  *month = -1;
  *day = -1;
  *year = -1;
}

extern void SetCalendarDate (
  DialoG d,
  Int4 month,
  Int4 day,
  Int4 year
)

{
  CalendarDate  cd;

  if (d == NULL) return;

  cd.month = month;
  cd.day = day;
  cd.year = year;

  PointerToDialog (d, (Pointer) &cd);
}

extern DialoG CreateCalendarDateDialog (
  GrouP prnt,
  Pointer userdata,
  DialogActnFunc actproc
)

{
  CalendarDate  cd;
  DatePagePtr   dpp;
  DayTime       dt;
  GrouP         f;
  GrouP         p;

  p = HiddenGroup (prnt, 1, 0, NULL);

  dpp = (DatePagePtr) MemNew (sizeof (DatePage));
  if (dpp) {

    SetObjectExtra (p, dpp, StdCleanupExtraProc);
    dpp->dialog = (DialoG) p;
    dpp->todialog = CalendarDatePtrToDatePage;
    dpp->fromdialog = DatePageToCalendarDatePtr;
    dpp->testdialog = NULL;

    dpp->actproc = actproc;
    dpp->userdata = userdata;

    f = HiddenGroup (p, -6, 0, NULL);

    dpp->month = PopupList (f, TRUE, ChangeDateProc);
    SetObjectExtra (dpp->month, (Pointer) dpp, NULL);
    InitEnumPopup (dpp->month, months_alist, NULL);
    SetValue (dpp->month, 1);

    dpp->day = PopupList (f, TRUE, ChangeDateProc);
    SetObjectExtra (dpp->day, (Pointer) dpp, NULL);
    InitEnumPopup (dpp->day, days_alist, NULL);
    SetValue (dpp->day, 1);

    StaticPrompt (f, ",", 0, popupMenuHeight, programFont, 'l');

    dpp->year = PopupList (f, TRUE, ChangeDateProc);
    SetObjectExtra (dpp->year, (Pointer) dpp, NULL);
    InitEnumPopup (dpp->year, years_alist, NULL);
    SetValue (dpp->year, 1);

    if (GetDayTime (&dt)) {
      cd.month = dt.tm_mon + 1;
      cd.day = dt.tm_mday;
      cd.year = dt.tm_year + 1900;
      CalendarDatePtrToDatePage ((DialoG) p, (Pointer) &cd);
    }
  }

  return (DialoG) p;
}

