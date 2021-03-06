/*  installer.cpp
*
*  Author:    Michael Schwarz
*  Copyright (C) 2011 Michael Schwarz
*  Edited by: Manuel Liebert
*
* GNUBLIN Installer
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "net.h"
#include "disk.h"
#include "archive.h"
#include "settings.h"
#include "progress.h"
#include "backup.h"
#include "installer.h"
#include "calc_md5.h"

#define mount_point "/tmp/SDCard"
#define mount_point_kernel "/tmp/SDCard/kernel"
#define mount_point_gnublin "/tmp/SDCard/gnublin"
#define extract_kernel_path "/tmp/SDCard/extract_kernel"
#define kernel_path "/tmp/SDCard/extract_kernel/zImage"
#define lib_path "/tmp/SDCard/extract_kernel/lib"
#define filePath "/usr/share/files/"

IMPLEMENT_APP(installer);

Window* frame;



bool installer::OnInit() {
  frame = new Window(0l, _("GNUBLIN Installer"));
  frame->Show();
  return true;
}

Window::Window(wxFrame* frame, const wxString& title) : wxFrame(frame, -1, title) {
  ReadURLs();

  this->SetSizeHints(640, 530);

  wxBoxSizer* main_sizer;
  main_sizer = new wxBoxSizer(wxVERTICAL);

  label_select_device = new wxStaticText(this, wxID_ANY, _("Select Device"), wxDefaultPosition, wxDefaultSize, 0);
  label_select_device->Wrap(-1);
  label_select_device->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString));

  main_sizer->Add(label_select_device, 0, wxALL, 5);

  listctrl_device = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 100), wxLC_REPORT);
  main_sizer->Add(listctrl_device, 0, wxALL | wxEXPAND, 5);

  check_repartition = new wxCheckBox(this, wxID_ANY, _("Re-Partition"), wxDefaultPosition, wxDefaultSize, 0);
  check_repartition->SetValue(false);
  main_sizer->Add(check_repartition, 0, wxALL, 5);

  line_1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  main_sizer->Add(line_1, 0, wxEXPAND | wxALL, 5);


  wxGridSizer* bl_krnl_sizer;
  bl_krnl_sizer = new wxGridSizer(1, 2, 0, 0);

  wxStaticBoxSizer* layout_bootloader;
  layout_bootloader = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Bootloader")), wxVERTICAL);

  boot_no_change = new wxRadioButton(this, wxID_ANY, _("do not change"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  layout_bootloader->Add(boot_no_change, 0, wxALL, 5);

  boot_net = new wxRadioButton(this, wxID_ANY, _("fetch from http://gnublin.org/"), wxDefaultPosition, wxDefaultSize, 0);
  layout_bootloader->Add(boot_net, 0, wxALL, 5);

  wxFlexGridSizer* sizer_boot;
  sizer_boot = new wxFlexGridSizer(1, 2, 0, 0);
  sizer_boot->AddGrowableCol(1);
  sizer_boot->SetFlexibleDirection(wxBOTH);
  sizer_boot->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  boot_file = new wxRadioButton(this, wxID_ANY, _("use file"), wxDefaultPosition, wxDefaultSize, 0);
  sizer_boot->Add(boot_file, 0, wxALL, 5);

  file_bootloader = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, _("Select a file"), wxT("*"), wxDefaultPosition, wxSize(-1, -1), wxFLP_DEFAULT_STYLE | wxFLP_FILE_MUST_EXIST | wxFLP_OPEN);
  sizer_boot->Add(file_bootloader, 0, wxALL | wxEXPAND, 5);

  layout_bootloader->Add(sizer_boot, 0, wxEXPAND, 5);

  bl_krnl_sizer->Add(layout_bootloader, 0, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer* layout_rootfs;
  layout_rootfs = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("RootFS")), wxVERTICAL);

  root_no_change = new wxRadioButton(this, wxID_ANY, _("do not change"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  layout_rootfs->Add(root_no_change, 0, wxALL, 5);

  root_net = new wxRadioButton(this, wxID_ANY, _("fetch from http://gnublin.org/"), wxDefaultPosition, wxDefaultSize, 0);
  layout_rootfs->Add(root_net, 0, wxALL, 5);

  wxFlexGridSizer* sizer_root;
  sizer_root = new wxFlexGridSizer(1, 2, 0, 0);
  sizer_root->AddGrowableCol(1);
  sizer_root->SetFlexibleDirection(wxBOTH);
  sizer_root->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  root_file = new wxRadioButton(this, wxID_ANY, _("use file"), wxDefaultPosition, wxDefaultSize, 0);
  sizer_root->Add(root_file, 0, wxALL, 5);

  file_rootfs = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, _("Select a file"), wxT("*"), wxDefaultPosition, wxSize(-1, -1), wxFLP_DEFAULT_STYLE | wxFLP_FILE_MUST_EXIST | wxFLP_OPEN);
  sizer_root->Add(file_rootfs, 0, wxALL | wxEXPAND, 5);

  layout_rootfs->Add(sizer_root, 0, wxEXPAND, 5);

  bl_krnl_sizer->Add(layout_rootfs, 0, wxEXPAND | wxALL, 5);

  main_sizer->Add(bl_krnl_sizer, 0, wxEXPAND, 5);

  wxGridSizer* krnl_board_sizer;
  krnl_board_sizer = new wxGridSizer(1, 2, 0, 0);

  wxStaticBoxSizer* layout_kernel;
  layout_kernel = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Kernel")), wxVERTICAL);

  kernel_no_change = new wxRadioButton(this, wxID_ANY, _("do not change"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  layout_kernel->Add(kernel_no_change, 0, wxALL, 5);

  krnl_net = new wxRadioButton(this, wxID_ANY, _("fetch from http://gnublin.org/"), wxDefaultPosition, wxDefaultSize, 0);
  layout_kernel->Add(krnl_net, 0, wxALL, 5);

  wxFlexGridSizer* sizer_kernel;
  sizer_kernel = new wxFlexGridSizer(1, 2, 0, 0);
  sizer_kernel->AddGrowableCol(1);
  sizer_kernel->SetFlexibleDirection(wxBOTH);
  sizer_kernel->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_ALL);

  krnl_file = new wxRadioButton(this, wxID_ANY, _("use file"), wxDefaultPosition, wxDefaultSize, 0);
  sizer_kernel->Add(krnl_file, 0, wxALL, 5);

  file_zimage = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, _("Select a file"), wxT("*"), wxDefaultPosition, wxSize(-1, -1), wxFLP_DEFAULT_STYLE | wxFLP_FILE_MUST_EXIST | wxFLP_OPEN);
  sizer_kernel->Add(file_zimage, 1, wxALL | wxEXPAND, 5);

  layout_kernel->Add(sizer_kernel, 0, wxEXPAND, 5);

  krnl_board_sizer->Add(layout_kernel, 0, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer* sizer_board;
  sizer_board = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Board type (Downloadfiles)")), wxVERTICAL);

  board_32mb = new wxRadioButton(this, wxID_ANY, _("32 MB"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  sizer_board->Add(board_32mb, 0, wxALL, 5);

  board_8mb = new wxRadioButton(this, wxID_ANY, _("8 MB"), wxDefaultPosition, wxDefaultSize, 0);
  sizer_board->Add(board_8mb, 0, wxALL, 5);

  krnl_board_sizer->Add(sizer_board, 0, wxALL | wxEXPAND, 5);

  main_sizer->Add(krnl_board_sizer, 0, wxEXPAND, 5);





  btn_apply = new wxStdDialogButtonSizer();
  btn_applyApply = new wxButton(this, wxID_APPLY);
  btn_apply->AddButton(btn_applyApply);
  btn_apply->Realize();
  main_sizer->Add(btn_apply, 1, wxEXPAND, 5);

  this->SetSizer(main_sizer);
  this->Layout();

  main_menu = new wxMenuBar(0);
  menu_installer = new wxMenu();
  wxMenuItem* mitem_settings;
  mitem_settings = new wxMenuItem(menu_installer, wxID_ANY, wxString(_("Settings")) , wxEmptyString, wxITEM_NORMAL);
  menu_installer->Append(mitem_settings);

  menu_installer->AppendSeparator();

  wxMenuItem* mitem_quit;
  mitem_quit = new wxMenuItem(menu_installer, wxID_ANY, wxString(_("Quit")) + wxT('\t') + wxT("Q"), wxEmptyString, wxITEM_NORMAL);
  menu_installer->Append(mitem_quit);

  main_menu->Append(menu_installer, _("Installer"));

  menu_backup = new wxMenu();
  mitem_create_backup = new wxMenuItem(menu_backup, wxID_ANY, wxString(_("Create Backup")) , wxEmptyString, wxITEM_NORMAL);
  menu_backup->Append(mitem_create_backup);

  mitem_restore_backup = new wxMenuItem(menu_backup, wxID_ANY, wxString(_("Restore Backup")) , wxEmptyString, wxITEM_NORMAL);
  menu_backup->Append(mitem_restore_backup);

  main_menu->Append(menu_backup, _("Backup"));

  menu_about = new wxMenu();
  wxMenuItem* mitem_about;
  mitem_about = new wxMenuItem(menu_about, wxID_ANY, wxString(_("About")) , wxEmptyString, wxITEM_NORMAL);
  menu_about->Append(mitem_about);

  main_menu->Append(menu_about, _("?"));

  this->SetMenuBar(main_menu);


  // create control list columns
  wxListItem col_dev;
  col_dev.SetId(0);
  col_dev.SetText(wxT("Device"));
  col_dev.SetWidth(170);
  listctrl_device->InsertColumn(0, col_dev);

  wxListItem col_size;
  col_size.SetId(1);
  col_size.SetText(wxT("Size"));
  col_size.SetWidth(100);
  listctrl_device->InsertColumn(1, col_size);

  wxListItem col_model;
  col_model.SetId(2);
  col_model.SetText(wxT("Model"));
  col_model.SetWidth(220);
  listctrl_device->InsertColumn(2, col_model);


  // read settings
  ReadSettings();

  // check if root
  if(!is_root() && checkroot) {
    wxMessageBox(_("You have to run this program as root!"), _("Error"), wxOK | wxICON_ERROR);
    Close();
    exit(0);
  }

  // get list of devices
  ReadDevices();

  // connect menu items
  this->Connect(mitem_quit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::OnQuit));
  this->Connect(mitem_about->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::ShowAbout));
  this->Connect(mitem_settings->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::ShowSettings));
  this->Connect(mitem_create_backup->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::CreateBackup));
  this->Connect(mitem_restore_backup->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Window::RestoreBackup));

  btn_applyApply->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Window::DoInstall), NULL, this);
  listctrl_device->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler(Window::DeviceDeselected), NULL, this);
  listctrl_device->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(Window::DeviceSelected), NULL, this);

  // disable gui
  EnableGUI(false);

}

void Window::ReadDevices() {
  // get list of drives
  char buffer[128];
  Drive** drives = get_devices();
  int id = 0;

  listctrl_device->DeleteAllItems();

  while(drives && *drives) {
    format_size(buffer, (*drives)->length * (*drives)->sector_size);

    // skip drives with size > 16 GB (change in settings)
    if((*drives)->length * (*drives)->sector_size / 1024 / 1024 > hidesize && hidedevice) {
      drives++;
      continue;
    }

    wxListItem item;
    item.SetId(id);
    item.SetText(wxString::FromAscii((*drives)->name));

    listctrl_device->InsertItem(item);
    listctrl_device->SetItem(id, 1, wxString::FromAscii(buffer));
    listctrl_device->SetItem(id, 2, wxString::FromAscii((*drives)->model));

    id++;
    drives++;
  }

  EnableGUI(false);
}


void Window::ReadSettings() {
  // default
  bootsector_size = 1024 * 1024 * 16; // 16 MB
  hidedevice = true;
  hidesize = 16 * 1024;  // 16 GB
  checkroot = true;

  wxXmlDocument doc;
  if(!doc.Load(wxT("settings.xml"))) {
    return;
  }

  // start processing the XML file
  if(doc.GetRoot()->GetName() != wxT("installer")) {
    return;
  }

  wxXmlNode* child = doc.GetRoot()->GetChildren();
  while(child) {

    // get settings
    if(child->GetName() == wxT("settings")) {
      wxXmlNode* s_child = child->GetChildren();

      while(s_child) {

        // get bootsector size
        if(s_child->GetName() == wxT("bootsector")) {
          wxString bs = s_child->GetNodeContent();
          printf("bootsector size: %s\n", C_STR(bs));

          long bs_size;
          if(!bs.ToLong(&bs_size)) {
            printf("Error reading bootsector size\n");
          } else {
            bootsector_size = bs_size * 1024 * 1024;
          }
        }

        // limit above which devices are not shown
        if(s_child->GetName() == wxT("hidesize")) {
          wxString hs = s_child->GetNodeContent();
          printf("hide if bigger than: %s\n", C_STR(hs));

          long hs_size;
          if(!hs.ToLong(&hs_size)) {
            printf("Error reading hidesize value\n");
          } else {
            hidesize = hs_size * 1024;
          }
        }

        // hide devices
        if(s_child->GetName() == wxT("hidedevice")) {
          wxString h = s_child->GetNodeContent();
          printf("hide devices: %s\n", C_STR(h));

          if(h == wxT("true")) {
            hidedevice = true;
          } else {
            hidedevice = false;
          }
        }

        // check if root
        if(s_child->GetName() == wxT("checkroot")) {
          wxString rt = s_child->GetNodeContent();
          printf("check for root: %s\n", C_STR(rt));

          if(rt == wxT("true")) {
            checkroot = true;
          } else {
            checkroot = false;
          }
        }

        s_child = s_child->GetNext();
      }
    }

    child = child->GetNext();
  }
}

void Window::WriteSettings() {
  wxXmlDocument doc;
  if(!doc.Load(wxT("settings.xml"))) {
    return;
  }

  // start processing the XML file
  if(doc.GetRoot()->GetName() != wxT("installer")) {
    return;
  }

  wxXmlNode* child = doc.GetRoot()->GetChildren();
  while(child) {

    // get settings
    if(child->GetName() == wxT("settings")) {
      wxXmlNode* s_child = child->GetChildren();

      while(s_child) {

        // bootsector size
        if(s_child->GetName() == wxT("bootsector")) {
          wxString bs = wxString::Format(wxT("%i"), bootsector_size / 1024 / 1024);
          s_child->GetChildren()->SetContent(bs);
        }

        // limit above which devices are not shown
        if(s_child->GetName() == wxT("hidesize")) {
          wxString hs = wxString::Format(wxT("%i"), hidesize / 1024);
          s_child->GetChildren()->SetContent(hs);
        }

        // hide devices
        if(s_child->GetName() == wxT("hidedevice")) {
          wxString h;
          if(hidedevice) {
            h = wxT("true");
          } else {
            h = wxT("false");
          }
          s_child->GetChildren()->SetContent(h);
        }

        // check if root
        if(s_child->GetName() == wxT("checkroot")) {
          wxString rt;
          if(checkroot) {
            rt = wxT("true");
          } else {
            rt = wxT("false");
          }
          s_child->GetChildren()->SetContent(rt);
        }

        s_child = s_child->GetNext();
      }
    }

    child = child->GetNext();
  }

  doc.Save(wxT("settings.xml"));
}

void Window::ReadURLs() {
  wxXmlDocument doc;
  if(!doc.Load(wxT("settings.xml"))) {
    return;
  }

  if(doc.GetRoot()->GetName() != wxT("installer")) {
    return;
  }

  wxXmlNode* child = doc.GetRoot()->GetChildren();
  while(child) {

    // get settings
    if(child->GetName() == wxT("url")) {
      wxXmlNode* s_child = child->GetChildren();

      while(s_child) {
        wxString board_file;

        // get kernel url
        if(s_child->GetName() == wxT("kernel_32mb")) {
          wxString k = s_child->GetNodeContent();
          printf("kernel 32mb url: %s\n", C_STR(k));

          url_kernel_32mb = k;
        }
        if(s_child->GetName() == wxT("kernel_8mb")) {
          wxString k = s_child->GetNodeContent();
          printf("kernel 8mb url: %s\n", C_STR(k));

          url_kernel_8mb = k;
        }

        // get bootloader url
        if(s_child->GetName() == wxT("bootloader_32mb")) {
          wxString b = s_child->GetNodeContent();
          printf("bootloader 32mb url: %s\n", C_STR(b));

          url_bootloader_32mb = b;
        }
        if(s_child->GetName() == wxT("bootloader_8mb")) {
          wxString b = s_child->GetNodeContent();
          printf("bootloader 8mb url: %s\n", C_STR(b));

          url_bootloader_8mb = b;
        }

        // get rootfs url
        if(s_child->GetName() == wxT("rootfs_32mb")) {
          wxString r = s_child->GetNodeContent();
          printf("rootfs 32mb url: %s\n", C_STR(r));

          url_rootfs_32mb = r;
        }
        if(s_child->GetName() == wxT("rootfs_8mb")) {
          wxString r = s_child->GetNodeContent();
          printf("rootfs 8mb url: %s\n", C_STR(r));

          url_rootfs_8mb = r;
        }

        // display
        if(s_child->GetName() == wxT("display")) {
          display_url = s_child->GetNodeContent();
          printf("url display: %s\n", C_STR(display_url));
        }

        s_child = s_child->GetNext();
      }
    }

    child = child->GetNext();
  }
}

void Window::EnableGUI(bool enabled) {
  check_repartition->Enable(enabled);
  boot_no_change->Enable(enabled);
  boot_net->Enable(enabled);
  boot_file->Enable(enabled);
  file_bootloader->Enable(enabled);
  krnl_net->Enable(enabled);
  krnl_file->Enable(enabled);
  file_zimage->Enable(enabled);
  board_8mb->Enable(enabled);
  board_32mb->Enable(enabled);
  root_no_change->Enable(enabled);
  kernel_no_change->Enable(enabled);
  root_net->Enable(enabled);
  root_file->Enable(enabled);
  file_rootfs->Enable(enabled);
  btn_applyApply->Enable(enabled);
  mitem_create_backup->Enable(enabled);
  mitem_restore_backup->Enable(enabled);
}

void Window::EnableTotalGUI(bool enabled) {
  EnableGUI(enabled);
  listctrl_device->Enable(enabled);
  main_menu->Enable(enabled);
}

void Window::DeviceSelected(wxListEvent& event) {
  EnableGUI(true);
}

void Window::DeviceDeselected(wxListEvent& event) {
  EnableGUI(false);
}

void Window::ShowSettings(wxCommandEvent& event) {
  SettingsFrame* s = new SettingsFrame(frame);
  s->Show();

  s->SetHideDev(hidedevice);
  s->SetHideDevSize(hidesize / 1024);
  s->SetCheckRoot(checkroot);
  s->SetBSSize(bootsector_size / 1024 / 1024);
}

void Window::StopBackup() {
  stop_backup = 1;
  printf("stopped!\n");
}

void Window::CopyDevice(FILE* in, FILE* out, long long kbytes, BackupProgress* b) {
  unsigned long blocksize = 1024 * 1024;
  char* buffer = (char*)malloc(blocksize);
  unsigned long total = 0;
  long long dev_size = kbytes;

  long start_time = time(NULL);

  while(!feof(in)) {
    if(fread(buffer, blocksize, 1, in) != blocksize) {
      printf("CopyDevice: ERROR read in file\n");
      wxMessageBox(_("ERROR: read in file!"), _("Error"), wxOK | wxICON_ERROR);
      return;
    }
    fwrite(buffer, blocksize, 1, out);
    total++;

    // calculate remaining time
    long time_delta = time(NULL) - start_time;
    int percent = (int)(100.f * total * (blocksize / 1024.f)) / dev_size;

    if(percent != 0) {
      //long total_time = (100 * time_delta / percent);
      long remaining_time = (time_delta / percent) * (100 - percent);

      wxString time_format;

      wxYield();
      if(stop_backup) {
        return;
      }
      b->SetRemainingTime(time_format.Format(_("%dm%02ds"), remaining_time / 60, remaining_time % 60));
    }

    wxYield();
    if(stop_backup) {
      return;
    }
    b->SetProgress(percent);
  }

  free(buffer);
}

void Window::CreateBackup(wxCommandEvent& event) {
  // check if a device is selected
  if(listctrl_device->GetSelectedItemCount() != 1) {
    return;
  }

  wxString device;

  wxFileDialog* saveFileDialog = new wxFileDialog(this, _("Save backup as..."), _(""), _(""),  _("Image|*.img"), wxSAVE, wxDefaultPosition);

  // create backup
  if(saveFileDialog->ShowModal() == wxID_OK) {

    stop_backup = 0;

    BackupProgress* b = new BackupProgress(frame);
    b->SetTextCreate();
    b->Show();
    EnableTotalGUI(false);

    Update();

    // start creating backup
    int itemIndex = listctrl_device->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    device = listctrl_device->GetItemText(itemIndex);
    long dev_size = get_device_size(C_STR(device)) / 1024; // device size in kbytes
    wxString image = saveFileDialog->GetFilename();

    if(dev_size == 0) {
      return;
    }

    FILE* in = fopen(C_STR(device), "rb");
    if(in == NULL) {
      wxMessageBox(_("Error opening device!"), _("Error"), wxOK | wxICON_ERROR);
      return;
    }

    FILE* out = fopen(C_STR(image), "wb");
    if(out == NULL) {
      wxMessageBox(_("Error opening file!"), _("Error"), wxOK | wxICON_ERROR);
      return;
    }

    CopyDevice(in, out, dev_size, b);

    b->Hide();
    b->Close();
    if(!stop_backup) {
      wxMessageBox(_("Creating backup was successful!"), _("Done"), wxOK);
    }

    fclose(in);
    fclose(out);
  }
}


void Window::RestoreBackup(wxCommandEvent& event) {
  // check if a device is selected
  if(listctrl_device->GetSelectedItemCount() != 1) {
    return;
  }

  // do you really want to do this, user?
  if(wxMessageBox(_("Do you really want to do this?\nThis destroys all the data on this device!"), _("Warning"), wxYES_NO | wxICON_EXCLAMATION | wxYES_DEFAULT) == wxNO) {
    return;
  }

  // get device
  wxString device;
  int itemIndex = listctrl_device->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

  device = listctrl_device->GetItemText(itemIndex);
  long dev_size = get_device_size(C_STR(device)) / 1024; // device size in kbytes

  // check if mounted
  int c;
  for(c = 0; c < 2; c++) {
    wxString part = device + wxString::FromAscii(c + 1 + '0');
    if(is_mounted(C_STR(part))) {
      unmount_partition(get_mountpoint(C_STR(part)));
      if(is_mounted(C_STR(part))) {
        wxMessageBox(_("Can not unmount '") + part + _("'!"), _("Error"), wxOK | wxICON_ERROR);
        return;
      }
    }
  }

  wxFileDialog* saveFileDialog = new wxFileDialog(this, _("Restore from..."), _(""), _(""),  _("Image|*.img"), wxOPEN | wxFILE_MUST_EXIST, wxDefaultPosition);

  // restore backup
  if(saveFileDialog->ShowModal() == wxID_OK) {
    wxString image = saveFileDialog->GetFilename();

    stop_backup = 0;

    BackupProgress* b = new BackupProgress(frame);
    b->SetTextRestore();
    b->Show();
    EnableTotalGUI(false);

    Update();

    // start creating backup


    if(dev_size == 0) {
      return;
    }

    FILE* out = fopen(C_STR(device), "wb");
    if(out == NULL) {
      wxMessageBox(_("Error opening device!"), _("Error"), wxOK | wxICON_ERROR);
      return;
    }

    FILE* in = fopen(C_STR(image), "rb");
    if(in == NULL) {
      wxMessageBox(_("Error opening file!"), _("Error"), wxOK | wxICON_ERROR);
      return;
    }

    CopyDevice(in, out, dev_size, b);

    b->Hide();
    b->Close();
    if(!stop_backup) {
      wxMessageBox(_("Restoring backup was successful!"), _("Done"), wxOK);
    }

    fclose(in);
    fclose(out);
  }
}

void Window::DoInstall(wxCommandEvent& event) {
  // check prerequisits
  if(listctrl_device->GetSelectedItemCount() != 1) {
    return;
  }

  // do you really want to do this, user?
  if(wxMessageBox(_("Do you really want to do this?\nThis may destroy all the data on this device!"), _("Warning"), wxYES_NO | wxICON_EXCLAMATION | wxYES_DEFAULT) == wxNO) {
    return;
  }


  InstallerFrame* i = new InstallerFrame(frame);
  i->Show();

  EnableTotalGUI(false);

  Update();
  // start the installation
  //i->AddLog(_("---- [ I N F O ] -----------"));

  // -- gather all user informations --
  wxString device;
  bool repartition;
  bool write_bootl;
  bool dl_bootl;
  bool file_bootl;
  wxString bootloader_file;
  bool dl_kernel;
  bool file_kernel;
  bool write_kernel;
  wxString kernel_file;
  bool dl_rootfs;
  bool write_rootfs;
  bool file_rfs;
  wxString rootfs_file;

  int parts = 0;

  // get device
  int itemIndex = listctrl_device->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  device = listctrl_device->GetItemText(itemIndex);
  // repartition?
  repartition = check_repartition->IsChecked();
  // write bootloader?
  write_bootl = !boot_no_change->GetValue();
  // download bootloader?
  dl_bootl = boot_net->GetValue();
  // bootloader file?
  file_bootl = boot_file->GetValue();
  if(file_bootl) {
    bootloader_file = file_bootloader->GetPath();
  }
  //write kernel?
  write_kernel = !kernel_no_change->GetValue();
  // download kernel?
  dl_kernel = krnl_net->GetValue();
  //kernel file?
  file_kernel = krnl_file->GetValue();
  if(file_kernel){
    kernel_file = file_zimage->GetPath();
  }
  // write rootfs?
  write_rootfs = !root_no_change->GetValue();
  // download rootfs?
  dl_rootfs = root_net->GetValue();
  // rootfs file?
  file_rfs = root_file->GetValue();
  if(file_rfs) {
    rootfs_file = file_rootfs->GetPath();
  }

  // check mounted, mount, copy kernel, unmount, sync
  parts = 5;
  if(dl_bootl) {
    parts++;
  }
  if(dl_kernel) {
    parts++;
  }
  if(dl_rootfs) {
    parts++;
  }
  if(repartition) {
    parts++;
  }
  if(write_bootl) {
    parts++;
  }
  if(write_rootfs) {
    parts++;
  }
  if(write_kernel) {
    parts++;
  }

  int prog_part = 100 / parts;
  int total_progress = 0;
  int ErrorFlag = 0;

  i->SetProgress(total_progress);
  i->AddLog(_("Starting installation... This might take a few minutes"));

  if(!ErrorFlag) {
    // check if mounted
    total_progress += prog_part;
    i->SetProgress(total_progress);

    wxString boot_part;
    wxString linux_part;
    wxString kernel_part;
    if(device.Contains(_("mmc"))) {
      boot_part = device + _("p2");
      linux_part = device + _("p3");
      kernel_part = device + _("p1");
    } else {
      boot_part = device + _("2");
      linux_part = device + _("3");
      kernel_part = device + _("1");
    }



    int c;
    for(c = 0; c < 3; c++) {
      wxString part;
      if(c == 0) {
        part = boot_part;
      } else if(c == 1) {
        part = kernel_part;
      } else if(c == 2) {
        part = linux_part;
      }


      if(is_mounted(C_STR(part))) {
        i->AddLog(_("Unmounting ") + part);
        std::cout << "Unmounting: " << C_STR(part) << std::endl;
        unmount_partition(get_mountpoint(C_STR(part)));

        if(is_mounted(C_STR(part))) {
          i->AddLog(_("ERROR: can not unmount ") + part);

          return;
        }
      }
    }
    // create partitions
    if(repartition) {
      i->AddLog(_("Creating partitions"));
      total_progress += prog_part;
      i->SetProgress(total_progress);

      create_partitions(C_STR(device), bootsector_size, C_STR(linux_part), C_STR(kernel_part));
    } else { //check if partitions are there
      bool partitionFault = false;
      std::ifstream ifile;
      ifile.open(C_STR(boot_part));
      if(ifile) {
        ifile.close(); //File exists
      } else { //file dont exist
        i->AddLog(_("ERROR: could not open bootloader partition"));
        partitionFault = true;
      }
      ifile.open(C_STR(linux_part));
      if(ifile) {
        ifile.close(); //File exists
      } else { //file dont exist
        i->AddLog(_("ERROR: could not open Linux partition"));
        partitionFault = true;
      }
      ifile.open(C_STR(kernel_part));
      if(ifile) {
        ifile.close(); //File exists
      } else { //file dont exist
        i->AddLog(_("ERROR: could not open kernel partition"));
        partitionFault = true;
      }
      if(partitionFault == true) {
        if(wxMessageBox(_("ERROR: can not open Linux/bootloader partition file!\nWould you like to repartition the SD-Card?"), \
                        _("Warning"), wxYES_NO | wxICON_QUESTION | wxYES_DEFAULT) == wxYES) {

          if((write_kernel == 0) | (write_rootfs == 0) | (write_bootl == 0)) {
            if(wxMessageBox(_("Bootloader, rootfs or kernel is selected not to change! Please click...\n 'Yes' to continue anyway (gnublin will not boot)! \n 'No' to cancel and make a different choice"), _("Warning"), wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT) == wxNO) {
              i->Close();
              return;
            }
          }
          i->AddLog(_("Creating partitions"));
          create_partitions(C_STR(device), bootsector_size, C_STR(linux_part), C_STR(kernel_part));
        } else {
          i->Close();
          return;
        }
      }
    }

    // download bootloader
    if(dl_bootl & !ErrorFlag) {
      mkdir(filePath, 0755);
      i->AddLog(_("Downloading and checking bootloader"));
      if(board_32mb->GetValue()) {
        if((ErrorFlag = ChecknLoad(_("apex.bin"), url_bootloader_32mb, i))) {
          i->AddLog(_("Error while load and md5 check bootloader file."));
          return;
        }
      }

      else if(board_8mb->GetValue()) {
        if((ErrorFlag = ChecknLoad(_("apex.bin"), url_bootloader_8mb, i))) {
          i->AddLog(_("Error while load and md5 check bootloader file."));
          return;
        }
      }
      total_progress += prog_part;
      i->SetProgress(total_progress);

      bootloader_file = (wxString)_(filePath) + _("apex.bin");
    }

    // download kernel
    if(dl_kernel & !ErrorFlag) {
      mkdir(filePath, 0755);
      i->AddLog(_("Downloading and checking kernel"));
      if(board_32mb->GetValue()) {
        if((ErrorFlag = ChecknLoad(_("kernel.tar.gz"), url_kernel_32mb, i))) {
          i->AddLog(_("Error while load and md5 check kernel file."));
          return;
        }
      } else if(board_8mb->GetValue()) {
        if((ErrorFlag = ChecknLoad(_("kernel.tar.gz"), url_kernel_8mb, i))) {
          i->AddLog(_("Error while load and md5 check kernel file."));
          return;
        }
      }

      total_progress += prog_part;
      i->SetProgress(total_progress);


      kernel_file = (wxString) _(filePath) + _("kernel.tar.gz");
    }

    // download rootfs
    if(dl_rootfs & !ErrorFlag) {
      mkdir(filePath, 0755);
      i->AddLog(_("Downloading and checking rootfs"));
      if(board_32mb->GetValue()) {
        if((ErrorFlag = ChecknLoad(_("rootfs.tar.gz"), url_rootfs_32mb, i))) {
          i->AddLog(_("Error while load and md5 check rootfs file."));
          return;
        }
      } else if(board_8mb->GetValue()) {
        if((ErrorFlag = ChecknLoad(_("rootfs.tar.gz"), url_rootfs_8mb, i))) {
          i->AddLog(_("Error while load and md5 check rootfs file."));
          return;
        }
      }

      total_progress += prog_part;
      i->SetProgress(total_progress);

      rootfs_file = (wxString) _(filePath) + _("rootfs.tar.gz");
    }


    // copy bootloader
    if(write_bootl) {
      i->AddLog(_("Writing bootloader"));
      total_progress += prog_part;
      i->SetProgress(total_progress);
      dd(C_STR(bootloader_file), C_STR(boot_part), 512);
    }

    if(write_rootfs | write_kernel) {
      // mount
      i->AddLog(_("Mounting Linux partition"));
      total_progress += prog_part;
      i->SetProgress(total_progress);

      mkdir(mount_point, 0777);
      mkdir(mount_point_kernel, 0777);
      mkdir(mount_point_gnublin, 0777);
      mkdir(extract_kernel_path, 0777);
      mount_partition(C_STR(linux_part), mount_point_gnublin);
      mount_partition(C_STR(kernel_part), mount_point_kernel);

      if(!is_mounted(C_STR(linux_part))) {
        i->AddLog(_("ERROR: can not mount gnublin filesystem"));
        return;
      }
      if(!is_mounted(C_STR(kernel_part))) {
        i->AddLog(_("ERROR: can not mount kernel filesystem"));
        return;
      }
      // extract rootfs
      if(write_rootfs) {
        i->AddLog(_("Extracting RootFS"));
        total_progress += prog_part;
        i->SetProgress(total_progress);

        extract_archive(C_STR(rootfs_file), mount_point_gnublin);
      }


      std::cout << "copy rootfs done! starting copy kernel" << std::endl;

      // copy kernel
      if(write_kernel) {
        i->AddLog(_("extracting kernel"));
        total_progress += prog_part;
        i->SetProgress(total_progress);
        extract_archive(C_STR(kernel_file), extract_kernel_path);
      }
      char* cp_in = (char*)kernel_path;
      char* cp_out = (char*)mount_point_kernel;
      char command[1000];
      sprintf(command, "cp %s %s", cp_in, cp_out);
      if(system(command) != 0) {
        printf("ERROR call cp systemcall to copy kernel...!\n");
      }

      cp_in = (char*) lib_path;
      cp_out = (char*) mount_point_gnublin;
      sprintf(command, "cp -r %s %s", cp_in, cp_out);
      if(system(command) != 0) {
        printf("ERROR call cp systemcall to copy lib...!\n");
      }

      std::cout << "copy kernel done! syncing ..." << std::endl;


      // sync
      i->AddLog(_("Syncing..."));
      total_progress += prog_part;
      i->SetProgress(total_progress);

      sync_card();

      std::cout << "sync done! unmount partition..." << std::endl;


      for(c = 0; c < 3; c++) {
        wxString part;
        if(device.Contains(_("mmc"))) {
          part = device + _("p") + wxString::FromAscii(c + 1 + '0');
        } else {
          part = device + wxString::FromAscii(c + 1 + '0');
        }
        sleep(4);
        if(is_mounted(C_STR(part))) {
          i->AddLog(_("Unmounting ") + part);
          std::cout << "Unmounting: " << C_STR(part) << std::endl;

          unmount_partition(get_mountpoint(C_STR(part)));

          if(is_mounted(C_STR(part))) {
            sleep(4);
            std::cout << "Unmounting: " << mount_point_gnublin << std::endl;
            unmount_partition(mount_point_gnublin);
            if(is_mounted(C_STR(part))) {
              i->AddLog(_("ERROR: can not unmount ") + part);
              std::cout << "ERROR: can not unmount " << C_STR(part);
              return;
            }
          }
        }
      }

      std::cout << "unmount done!" << std::endl;
    }


    i->AddLog(_("Done!"));
  }
  i->AddLog(_("You may close this window now."));

  i->SetProgress(100);
}

int Window::ChecknLoad(wxString file, wxString url, InstallerFrame* i) {

  wxString md5file;
  wxString md5calc;
  i->AddLog(_("Downloading ") + file + _(".md5"));
  get_file((C_STR(url + _(".md5"))), C_STR(_(filePath) + file + _(".md5")));

  FILE* file_bl = fopen(C_STR(_(filePath) + file), "rb");
  if(file_bl == NULL) {
    i->AddLog(_("Downloading ") + file);
    get_file(C_STR(url), C_STR(_(filePath) + file));
  } else {
    fclose(file_bl);
  }

  wxFileInputStream file_md5(_(filePath) + file + _(".md5"));
  if(!file_md5) {
    //could not open file!
  }

  wxTextInputStream tfile_md5(file_md5);
  md5file = tfile_md5.ReadLine();
  if(md5file.Contains(_("DOCTYPE HTML PUBLIC"))) {
    i->AddLog(_("md5 file not found on the server"));
    std::cout << "md5 file not found on the server" << std::endl;
    wxMessageBox(_("Error while getting the ") + file + _(".md5 file! Please check the links in the settings.xml file or contact us on gnublin.org"), _("Error"), wxOK | wxICON_ERROR);
    return -1;
  } else if(md5file.Len() == 0) {
    i->AddLog(_("could not load md5 file"));
    std::cout << "could not load md5 file" << std::endl;
    wxMessageBox(_("Error while getting the ") + file + _(".md5 file! Perhaps there is no internet connection?"), _("Error"), wxOK | wxICON_ERROR);
    return -1;
  } else {

    std::cout << "md5file: " << C_STR(md5file) << std::endl;
    md5calc = wxString::FromAscii(calc_md5(C_STR(_(filePath) + file)));
    std::cout << "md5calc: " << C_STR(md5calc) << std::endl;
    if(md5file.Contains(md5calc)) {
      i->AddLog(_("md5 check: ok"));
      std::cout << "md5 check: ok" << std::endl;
    } else {
      i->AddLog(_("md5 check: failed (old or corrupt file) -> redownload..."));
      std::cout << "md5 check: failed (old or corrupt file) -> redownload..." << std::endl;
      get_file(C_STR(url), C_STR(_(filePath) + file));
      md5calc = wxString::FromAscii(calc_md5(C_STR(_(filePath) + file)));
      std::cout << "md5calc: " << C_STR(md5calc) << std::endl;
      if(md5file.Contains(md5calc)) {
        i->AddLog(_("md5 check: ok"));
        std::cout << "md5 check: ok" << std::endl;
      } else {
        i->AddLog(_("md5 check: failed again"));
        std::cout << "md5 check: failed again" << std::endl;
        if(wxMessageBox(_("Error while getting/md5-checking the ") + file + _(" file! Perhaps there is no internet connection? For further information check the command line. Continue anyway?(Absolutely not recomended!)"), _("Error"), wxYES_NO | wxICON_EXCLAMATION | wxNO_DEFAULT) == wxNO) {
          return -1;
        } else {
          return 0;
        }
      }
    }
  }
  return 0;
}

void Window::SetHideDev(bool h) {
  hidedevice = h;
}

void Window::SetHideDevSize(unsigned long s) {
  hidesize = s * 1024;
}

void Window::SetCheckRoot(bool c) {
  checkroot = c;
}

void Window::SetBSSize(unsigned long s) {
  bootsector_size = s * 1024 * 1024;
}


void Window::ShowAbout(wxCommandEvent& event) {
  wxAboutDialogInfo info;
  info.SetName(_("GNUBLIN Installer"));
  info.SetVersion(_(VERSION));
  info.SetDescription(_("Installer for GNUBLIN Linux Board"));
  info.SetCopyright(wxT("(C) 2011 Michael Schwarz <michael.schwarz91@gmail.com>\nEdited by Manuel Liebert <man.liebert@gmail.com>"));

  wxAboutBox(info);
}


void Window::OnQuit(wxCommandEvent& event) {
  Close();
}




