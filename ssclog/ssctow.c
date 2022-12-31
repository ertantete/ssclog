/*
    SKYLINE SOARING CLUB DUTY LOGSHEET PROGRAM - Towplane page

    Copyright (c) 2004-2011 Jonathan A. Kans. All rights reserved.

    Ver   Date       Author           Comment
    ----- ---------- ---------------- ------------------------------------------
    1.0              Jonathan Kans    Created
    1.1   12/21/2022 Ertan Tete       Sanitize/validate towplane tach time and
                                      gas added data - make sure entered data
				      represents decimal numbers.

*/

#include "ssclog.h"

/* dialog controls */

typedef struct ssctowplane {
  DIALOG_MESSAGE_BLOCK

  PopuP         towplane;
  TexT          start_tach;
  TexT          end_tach;
  PrompT        tach_time;
  TexT          gas_added;
  TexT          comment;

  SscTablesPtr  tables;
} SscTowplane, PNTR SscTowplanePtr;

static Int4 ParseTachTimeTimes100 (
  CharPtr str
)

{
  Char      buf [32];
  Int4      ones, tens;
  CharPtr   ptr;
  long int  val;

  if (StringHasNoText (str)) return 0;

  StringCpy (buf, str);
  ptr = StringChr (buf, '.');
  if (ptr != NULL) {
    *ptr = '\0';
    ptr++;
    if (sscanf (buf, "%ld", &val) == 1) {
      tens = (Int4) val;
      ptr [2] = '\0';
      if (StringDoesHaveText (ptr)) {
        if (ptr [1] == '\0') {
          ptr [1] = '0';
        }
        if (sscanf (ptr, "%ld", &val) == 1) {
          ones = (Int4) val;
          //if (tens > 0) {
	  if (tens >= 0) {
            return tens * 100L + ones;
          }
        }
      } else {
        if (tens > 0) {
          return tens * 100L;
        }
      }
    }
  } else {
    if (sscanf (buf, "%ld", &val) == 1) {
      tens = (Int4) val;
      if (tens > 0) {
        return tens * 100L;
      }
    }
  }

  return 0;
}

static void NormalizeTachTime (
  TexT t
)

{
  Char     buf [64], cpy [64];
  Char     ch;
  Boolean  go_on = TRUE, need_to_update = FALSE;
  Int2     leading_digits = 0, num_periods = 0, trailing_digits = 0;
  CharPtr  ptr;

  GetTitle (t, buf, sizeof (buf));
  if (IS_WHITESP (buf [0])) {
    TrimSpacesAroundString (buf);
    need_to_update = TRUE;
  }

  ptr = buf;
  ch = *ptr;
  while (ch != '\0' && go_on) {
    if (IS_DIGIT (ch)) {
      if (num_periods > 0) {
        trailing_digits++;
        if (trailing_digits > 2) {
          *ptr = '\0';
          need_to_update = TRUE;
          go_on = FALSE;
        }
      } else {
        leading_digits++;
      }
    } else if (ch == '.' || ch == ',') {
      if (ch == ',') {
        *ptr = '.';
        need_to_update = TRUE;
      }
      num_periods++;
      //if (num_periods > 1 && leading_digits == 4) {
      if (num_periods > 1) {
	strcpy(ptr, ptr + 1); //*ptr = '\0';
        need_to_update = TRUE;
	num_periods--;
	ch = *ptr;
	continue;
        //go_on = FALSE;
      }
    } else {
        strcpy(ptr, ptr + 1); //*ptr = '\0';
        need_to_update = TRUE;
	ch = *ptr;
	continue;
        //go_on = FALSE;
    }
    ptr++;
    ch = *ptr;
  }

  if (num_periods == 0 && leading_digits == 6) {
    StringCpy (cpy, buf);
    buf [7] = '\0';
    buf [6] = buf [5];
    buf [5] = buf [4];
    buf [4] = '.';
    need_to_update = TRUE;
  }

  if (strlen(buf) > 11) {
    buf[11] = '\0';
    need_to_update = TRUE;
  }

  if (need_to_update) {
    SetTitle (t, buf);
  }
}

static Boolean CalculateTachTime (
  TexT start_tach,
  TexT end_tach,
  CharPtr tach_time
)

{
  Char     buf [64];
  Char     ch;
  size_t   len;
  Int4     start, end, time;

  if (start_tach == NULL || end_tach == NULL || tach_time == NULL) return FALSE;

  NormalizeTachTime (start_tach);
  NormalizeTachTime (end_tach);

  GetTitle (start_tach, buf, sizeof (buf));
  ch = buf [0];
  if (! IS_DIGIT (ch)) return FALSE;
  if (StringChr (buf, '.') == NULL) return FALSE;
  start = ParseTachTimeTimes100 (buf);

  GetTitle (end_tach, buf, sizeof (buf));
  ch = buf [0];
  if (! IS_DIGIT (ch)) return FALSE;
  if (StringChr (buf, '.') == NULL) return FALSE;
  end = ParseTachTimeTimes100 (buf);

  time = end - start;
  if (time > 0) {
    sprintf (buf, "%ld", (long) time);
    len = StringLen (buf);
    if (len == 1) {
      buf [2] = '\0';
      buf [1] = buf [0];
      buf [0] = '0';
      len = StringLen (buf);
    }
    if (len > 1) {
      buf [len + 1] = '\0';
      buf [len] = buf [len - 1];
      buf [len - 1] = buf [len - 2];
      buf [len - 2] = '.';
      StringCat (buf, " ");
      StringCpy (tach_time, buf);
      return TRUE;
    }
  }

  return FALSE;
}

static void RecalculateTachTime (
  TexT t
)

{
  Char            buf [64];
  SscTowplanePtr  spp;

  spp = (SscTowplanePtr) GetObjectExtra (t);
  if (spp == NULL) return;

  if (CalculateTachTime (spp->start_tach, spp->end_tach, buf)) {
    SafeSetTitle (spp->tach_time, buf);
  } else {
    SafeSetTitle (spp->tach_time, "");
  }
}

static void NormalizeGasAdded (
  TexT t
)

{
  SscTowplanePtr  spp;

  spp = (SscTowplanePtr) GetObjectExtra (t);
  if (spp == NULL) return;

  // Reuse function NormalizeTachTime to sanitize gas_added data - limit
  // input to digits, ".", and ",", replace "," with ".", allow max one ".",
  // i.e. make sure the gas_added string represents a valid number.
  // Note: 
  // - Function ValidateLogsheet in sscmain.c checks that gas_added starts 
  // and ends with digit and  contains "."
  NormalizeTachTime(spp->gas_added);
}


static void PrintText (TexT t, Boolean tab, FILE *fp)

{
  CharPtr  str;

  str = SaveStringFromText (t);
  if (tab) {
    fprintf (fp, "\t");
  }
  if (StringDoesHaveText (str)) {
    TrimSpacesAroundString (str);
    fprintf (fp, "%s", str);
  }
  MemFree (str);
}

static void TowplaneInfoPtrToTowplanePage (
  DialoG d,
  Pointer data
)

{
  Char             buf [64];
  SscTowplanePtr   spp;
  SscTablesPtr     stp;
  TowPlaneDataPtr  tpp;

  spp = (SscTowplanePtr) GetObjectExtra (d);
  if (spp == NULL) return;
  tpp = (TowPlaneDataPtr) data;

  if (tpp == NULL) {
    SetValue (spp->towplane, 0);
    SetTitle (spp->start_tach, "");
    SetTitle (spp->end_tach, "");
    SetTitle (spp->tach_time, "");
    SetTitle (spp->gas_added, "");
    SetTitle (spp->comment, "");
    return;
  }

  stp = spp->tables;
  if (stp == NULL) return;

  if (StringDoesHaveText (tpp->towplane)) {
    SetEnumPopupByName (spp->towplane, stp->towplane_alist, tpp->towplane);
  } else {
    SetValue (spp->towplane, 0);
  }
  SetTitle (spp->start_tach, tpp->start_tach);
  SetTitle (spp->end_tach, tpp->end_tach);
  if (CalculateTachTime (spp->start_tach, spp->end_tach, buf)) {
    SetTitle (spp->tach_time, buf);
  } else {
    SetTitle (spp->tach_time, "");
  }
  SetTitle (spp->gas_added, tpp->gas_added);
  
  SetTitle (spp->comment, tpp->comment);

  Update ();
}

static Pointer TowplanePageToTowplaneInfoPtr (
  DialoG d
)

{
  MsgAnswer        ans;
  Char             buf [64];
  SscTowplanePtr   spp;
  SscTablesPtr     stp;
  CharPtr          tmp;
  TowPlaneDataPtr  tpp;

  spp = (SscTowplanePtr) GetObjectExtra (d);
  if (spp == NULL) return NULL;

  stp = spp->tables;
  if (stp == NULL) return NULL;

  tpp = TowPlaneDataNew ();
  if (tpp == NULL) return NULL;

  tpp->towplane = GetEnumPopupByName (spp->towplane, stp->towplane_alist);
  tpp->start_tach = SaveStringFromText (spp->start_tach);
  tpp->end_tach = SaveStringFromText (spp->end_tach);
  if (CalculateTachTime (spp->start_tach, spp->end_tach, buf)) {
    TrimSpacesAroundString (buf);
    tpp->tach_time = StringSave (buf);
  } else if (CalculateTachTime (spp->end_tach, spp->start_tach, buf)) {
    ans = Message (MSG_YN, "Swap start and end tach times?");
    if (ans == ANS_YES) {
      tmp = tpp->start_tach;
      tpp->start_tach = tpp->end_tach;
      tpp->end_tach = tmp;
      TrimSpacesAroundString (buf);
      tpp->tach_time = StringSave (buf);
      SafeSetTitle (spp->tach_time, buf);
      SafeSetTitle (spp->start_tach, tpp->start_tach);
      SafeSetTitle (spp->end_tach, tpp->end_tach);
    }
  }
  tpp->gas_added = SaveStringFromText (spp->gas_added);

  tpp->comment = SaveStringFromText (spp->comment);
  FixNonPrintableChars (tpp->comment);

  if (StringHasNoText (tpp->towplane) &&
      StringHasNoText (tpp->start_tach) &&
      StringHasNoText (tpp->end_tach) &&
      StringHasNoText (tpp->tach_time) &&
      StringHasNoText (tpp->gas_added) &&
      StringHasNoText (tpp->comment)) {
    tpp = TowPlaneDataFree (tpp);
  }

  return (Pointer) tpp;
}

static DialoG CreateOneTowplaneDialog (
  GrouP prnt,
  SscTablesPtr stp,
  CharPtr title
)

{
  GrouP           c, g, p;
  FonT            fnt;
  SscTowplanePtr  spp;

  spp = (SscTowplanePtr) MemNew (sizeof (SscTowplane));
  if (spp == NULL) return NULL;

  p = HiddenGroup (prnt, -1, 0, NULL);
  SetGroupSpacing (p, 10, 10);

  SetObjectExtra (p, spp, StdCleanupExtraProc);
  spp->dialog = (DialoG) p;
  spp->todialog = TowplaneInfoPtrToTowplanePage;
  spp->fromdialog = TowplanePageToTowplaneInfoPtr;

  spp->tables = stp;

  g = HiddenGroup (p, 0, 2, NULL);
  SetGroupSpacing (g, 10, 5);
  SetGroupMargins (g, 5, 5);

#ifdef WIN_MSWIN
  fnt = systemFont;
#else
  fnt = programFont;
#endif

  StaticPrompt (g, title, 0, popupMenuHeight, fnt, 'l');
  spp->towplane = CreateEnumPopupDialog (g, TRUE, NULL, stp->towplane_alist, (UIEnum) 0, spp);

  StaticPrompt (g, "Start Tach", 0, dialogTextHeight, fnt, 'l');
  spp->start_tach =  DialogText (g, "00000.00", 0, RecalculateTachTime);
  SetObjectExtra (spp->start_tach, spp, NULL);

  StaticPrompt (g, "End Tach", 0, dialogTextHeight, fnt, 'l');
  spp->end_tach =  DialogText (g, "00000.00", 0, RecalculateTachTime);
  SetObjectExtra (spp->end_tach, spp, NULL);

  StaticPrompt (g, "Tach Time", 0, dialogTextHeight, fnt, 'l');
  spp->tach_time =  StaticPrompt (g, "000.00", 0, dialogTextHeight, systemFont, 'r');

  StaticPrompt (g, "Gas Added", 0, dialogTextHeight, fnt, 'l');
  spp->gas_added =  DialogText (g, "00000.0", 0, NormalizeGasAdded); //DialogText (g, "00000.0", 0, NULL);
  SetObjectExtra (spp->gas_added, spp, NULL); // ET

  c = HiddenGroup (p, 1, 0, NULL);
  StaticPrompt (c, "Comment", 0, dialogTextHeight, fnt, 'c');
  spp->comment =  ScrollText (c, 20, 3, programFont, TRUE, NULL);

  SetTitle (spp->towplane, "");
  SetTitle (spp->start_tach, "");
  SetTitle (spp->end_tach, "");
  SetTitle (spp->tach_time, "");
  SetTitle (spp->gas_added, "");
  SetTitle (spp->comment, "");

  AlignObjects (ALIGN_CENTER, (HANDLE) g, (HANDLE) c, NULL);

  return (DialoG) p;
}

typedef struct ssctowset {
  DIALOG_MESSAGE_BLOCK

  DialoG  first;
  DialoG  second;
} SscTowSet, PNTR SscTowSetPtr;

static CharPtr archiveTowplaneLabel = "Tow Plane            Start      End    Tach   Gas";

static void ArchiveOneTowplane (
  DialoG d,
  FILE *fp
)

{
  Char            buf [64];
  SscTowplanePtr  spp;
  SscTablesPtr    stp;
  CharPtr         str;

  spp = (SscTowplanePtr) GetObjectExtra (d);
  if (spp == NULL || fp == NULL) return;
  stp = spp->tables;
  if (stp == NULL) return;

  str = GetEnumPopupByName (spp->towplane, stp->towplane_alist);
  StringNCpy_0 (buf, str, sizeof (buf) - 20);
  StringCat (buf, "                    ");
  buf [20] = '\0';
  fprintf (fp, "%s", buf);

  PrintText (spp->start_tach, FALSE, fp);
  fprintf (fp, "  ");
  PrintText (spp->end_tach, FALSE, fp);
  fprintf (fp, "  ");
  if (CalculateTachTime (spp->start_tach, spp->end_tach, buf)) {
    TrimSpacesAroundString (buf);
    if (buf [1] == '.') {
      fprintf (fp, " ");
    }
    fprintf (fp, "%s", buf);
  }
  fprintf (fp, "  ");
  PrintText (spp->gas_added, FALSE, fp);
  fprintf (fp, "\n");
}

extern void ArchiveTowplaneDialog (
  DialoG d,
  FILE *fp
)

{
  SscTowSetPtr    ssp;

  ssp = (SscTowSetPtr) GetObjectExtra (d);
  if (ssp == NULL || fp == NULL) return;

  fprintf (fp, "TOWPLANE DATA -----\n\n%s\n\n", archiveTowplaneLabel);

  ArchiveOneTowplane (ssp->first, fp);
  ArchiveOneTowplane (ssp->second, fp);

  fprintf (fp, "\n");
}

static void TowplaneInfoPtrToTowSetPage (
  DialoG d,
  Pointer data
)

{
  SscTowSetPtr     ssp;
  TowPlaneDataPtr  tpp;

  ssp = (SscTowSetPtr) GetObjectExtra (d);
  if (ssp == NULL) return;
  tpp = (TowPlaneDataPtr) data;

  PointerToDialog (ssp->first, tpp);
  if (tpp != NULL) {
    tpp = tpp->next;
  }
  PointerToDialog (ssp->second, tpp);
}

static Pointer TowSetPageToTowplaneInfoPtr (
  DialoG d
)

{
  SscTowSetPtr     ssp;
  TowPlaneDataPtr  tpp1, tpp2;

  ssp = (SscTowSetPtr) GetObjectExtra (d);
  if (ssp == NULL) return NULL;

  tpp1 = DialogToPointer (ssp->first);
  tpp2 = DialogToPointer (ssp->second);

  if (tpp1 != NULL) {
    tpp1->next = tpp2;
    return (Pointer) tpp1;
  }

  return (Pointer) tpp2;
}

extern DialoG CreateTowplaneDialog (
  GrouP prnt,
  SscTablesPtr stp
)

{
  GrouP         g, p;
  SscTowSetPtr  ssp;

  ssp = (SscTowSetPtr) MemNew (sizeof (SscTowSet));
  if (ssp == NULL) return NULL;

  p = HiddenGroup (prnt, -1, 0, NULL);
  SetGroupSpacing (p, 10, 10);

  SetObjectExtra (p, ssp, StdCleanupExtraProc);
  ssp->dialog = (DialoG) p;
  ssp->todialog = TowplaneInfoPtrToTowSetPage;
  ssp->fromdialog = TowSetPageToTowplaneInfoPtr;

  g = HiddenGroup (p, 0, 2, NULL);
  SetGroupSpacing (g, 10, 5);
  SetGroupMargins (g, 5, 5);

  ssp->first = CreateOneTowplaneDialog (g, stp, "Tow Plane 1");
  ssp->second = CreateOneTowplaneDialog (g, stp, "Tow Plane 2");

  return (DialoG) p;
}

typedef struct sscmaintow {
  DIALOG_MESSAGE_BLOCK

  PopuP         towplane;

  SscTablesPtr  tables;
} MainTow, PNTR MainTowPtr;

static void ChangeTowName (
  PopuP p
)

{
  AlistDialogPtr   adp;
  MainTowPtr       mtp;
  SscTablesPtr     stp;
  CharPtr          str;
  TowPlaneListPtr  tlp;
  TowRateTablePtr  towrate = NULL;

  adp = (AlistDialogPtr) GetObjectExtra (p);
  if (adp == NULL) return;
  mtp = (MainTowPtr) adp->userdata;
  if (mtp == NULL) return;
  stp = mtp->tables;
  if (stp == NULL) return;

  str = GetEnumPopupByName (mtp->towplane, stp->towplane_alist);
  if (StringDoesHaveText (str)) {
    for (tlp = stp->towset_table; tlp != NULL; tlp = tlp->next) {
      if (StringICmp (tlp->towplane, str) == 0) {
        towrate = tlp->rates;
      }
    }
  }
  str = MemFree (str);

  if (towrate != NULL) {
    stp->towrate_table = towrate;
  } else {
    tlp = stp->towset_table;
    if (tlp != NULL) {
      stp->towrate_table = tlp->rates;
    }
  }

  SetDirtyFlag (mtp->tables);
}

static void TowPlaneNameToMainTowPage (DialoG d, Pointer data)

{
  MainTowPtr    mtp;
  SscTablesPtr  stp;
  CharPtr       str;

  mtp = (MainTowPtr) GetObjectExtra (d);
  if (mtp == NULL) return;
  stp = mtp->tables;
  if (stp == NULL) return;
  str = (CharPtr) data;

  if (StringDoesHaveText (str)) {
    SetEnumPopupByName (mtp->towplane, stp->towplane_alist, str);
  } else {
    SetValue (mtp->towplane, 0);
  }

  ChangeTowName (mtp->towplane);

  Update ();
}

static Pointer MainTowPageToTowPlaneName (DialoG d)

{
  MainTowPtr    mtp;
  SscTablesPtr  stp;
  CharPtr       str;

  mtp = (MainTowPtr) GetObjectExtra (d);
  if (mtp == NULL) return NULL;
  stp = mtp->tables;
  if (stp == NULL) return NULL;

  str = GetEnumPopupByName (mtp->towplane, stp->towplane_alist);
  if (StringHasNoText (str)) {
    str = MemFree (str);
  }

  return (Pointer) str;
}

extern DialoG CreateMainTowDialog (
  GrouP prnt,
  SscTablesPtr stp
)

{
  FonT        fnt;
  GrouP       g, p;
  MainTowPtr  mtp;

  mtp = (MainTowPtr) MemNew (sizeof (MainTow));
  if (mtp == NULL) return NULL;

  p = HiddenGroup (prnt, -1, 0, NULL);
  SetGroupSpacing (p, 10, 10);

  SetObjectExtra (p, mtp, StdCleanupExtraProc);
  mtp->dialog = (DialoG) p;
  mtp->todialog = TowPlaneNameToMainTowPage;
  mtp->fromdialog = MainTowPageToTowPlaneName;

  mtp->tables = stp;

  g = HiddenGroup (p, 2, 0, NULL);
  SetGroupSpacing (g, 10, 5);
  SetGroupMargins (g, 5, 5);

#ifdef WIN_MSWIN
  fnt = systemFont;
#else
  fnt = programFont;
#endif

  StaticPrompt (g, "Primary Tow Plane", 0, popupMenuHeight, fnt, 'l');
  mtp->towplane = CreateEnumPopupDialog (g, TRUE, ChangeTowName, stp->towplane_alist, (UIEnum) 0, mtp);

  return (DialoG) p;
}

