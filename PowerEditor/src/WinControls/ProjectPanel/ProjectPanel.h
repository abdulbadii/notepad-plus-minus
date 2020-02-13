// This file is part of Notepad++ project
// Copyright (C)2003 Don HO <don.h@free.fr>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Note that the GPL places important restrictions on "derived works", yet
// it does not provide a detailed definition of that term.  To avoid      
// misunderstandings, we consider an application to constitute a          
// "derivative work" for the purpose of this license if it does any of the
// following:                                                             
// 1. Integrates source code from Notepad++.
// 2. Integrates/includes/aggregates Notepad++ into a proprietary executable
//    installer, such as those produced by InstallShield.
// 3. Links to a library or executes a program that does any of the above.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#pragma once

#include "DockingDlgInterface.h"
#include "TreeView.h"
#include "ProjectPanel_rc.h"

#define PM_PROJECTPANELTITLE     L"Project"
#define PM_WORKSPACEROOTNAME     L"Workspace"
#define PM_NEWFOLDERNAME         L"Folder Name"
#define PM_NEWPROJECTNAME        L"Project Name"

#define PM_NEWWORKSPACE            L"New Workspace"
#define PM_OPENWORKSPACE           L"Open Workspace"
#define PM_RELOADWORKSPACE         L"Reload Workspace"
#define PM_SAVEWORKSPACE           L"Save"
#define PM_SAVEASWORKSPACE         L"Save As..."
#define PM_SAVEACOPYASWORKSPACE    L"Save a Copy As..."
#define PM_NEWPROJECTWORKSPACE     L"Add New Project"

#define PM_EDITRENAME              L"Rename"
#define PM_EDITNEWFOLDER           L"Add Folder"
#define PM_EDITADDFILES            L"Add Files..."
#define PM_EDITADDFILESRECUSIVELY  L"Add Files from Directory..."
#define PM_EDITREMOVE              L"Remove\tDEL"
#define PM_EDITMODIFYFILE          L"Modify File Path"

#define PM_WORKSPACEMENUENTRY      L"Workspace"
#define PM_EDITMENUENTRY           L"Edit"

#define PM_MOVEUPENTRY             L"Move Up\tCtrl+Up"
#define PM_MOVEDOWNENTRY           L"Move Down\tCtrl+Down"

enum NodeType {
	nodeType_root = 0, nodeType_project = 1, nodeType_folder = 2, nodeType_file = 3
};

class TiXmlNode;
class FileDialog;

class ProjectPanel : public DockingDlgInterface {
public:
	ProjectPanel(): DockingDlgInterface(IDD_PROJECTPANEL) {};


	void init(HINSTANCE hInst, HWND hPere) {
		DockingDlgInterface::init(hInst, hPere);
	}

    virtual void display(bool toShow = true) const {
        DockingDlgInterface::display(toShow);
    };

    void setParent(HWND parent2set){
        _hParent = parent2set;
    };

	void newWorkSpace();
	bool openWorkSpace(const TCHAR *projectFileName);
	bool saveWorkSpace();
	bool saveWorkSpaceAs(bool saveCopyAs);
	void setWorkSpaceFilePath(const TCHAR *projectFileName){
		_workSpaceFilePath = projectFileName;
	};
	const TCHAR * getWorkSpaceFilePath() const {
		return _workSpaceFilePath.c_str();
	};
	bool isDirty() const {
		return _isDirty;
	};
	void checkIfNeedSave(const TCHAR *title);

	virtual void setBackgroundColor(COLORREF bgColour) {
		TreeView_SetBkColor(_treeView.getHSelf(), bgColour);
    };
	virtual void setForegroundColor(COLORREF fgColour) {
		TreeView_SetTextColor(_treeView.getHSelf(), fgColour);
    };

protected:
	TreeView _treeView;
	HIMAGELIST _hImaLst = nullptr;
	HWND _hToolbarMenu = nullptr;
	HMENU _hWorkSpaceMenu = nullptr;
	HMENU _hProjectMenu = nullptr;
	HMENU _hFolderMenu = nullptr;
	HMENU _hFileMenu = nullptr;
	generic_string _workSpaceFilePath;
	generic_string _selDirOfFilesFromDirDlg;
	bool _isDirty = false;

	void initMenus();
	void destroyMenus();
	BOOL setImageList(int root_clean_id, int root_dirty_id, int project_id, int open_node_id, int closed_node_id, int leaf_id, int ivalid_leaf_id);
	void addFiles(HTREEITEM hTreeItem);
	void addFilesFromDirectory(HTREEITEM hTreeItem);
	void recursiveAddFilesFrom(const TCHAR *folderPath, HTREEITEM hTreeItem);
	HTREEITEM addFolder(HTREEITEM hTreeItem, const TCHAR *folderName);

	bool writeWorkSpace(TCHAR *projectFileName = NULL);
	generic_string getRelativePath(const generic_string & fn, const TCHAR *workSpaceFileName);
	void buildProjectXml(TiXmlNode *root, HTREEITEM hItem, const TCHAR* fn2write);
	NodeType getNodeType(HTREEITEM hItem);
	void setWorkSpaceDirty(bool isDirty);
	void popupMenuCmd(int cmdID);
	POINT getMenuDisplayPoint(int iButton);
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	bool buildTreeFrom(TiXmlNode *projectRoot, HTREEITEM hParentItem);
	void notified(LPNMHDR notification);
	void showContextMenu(int x, int y);
	void showContextMenuFromMenuKey(HTREEITEM selectedItem, int x, int y);
	HMENU getMenuHandler(HTREEITEM selectedItem);
	generic_string getAbsoluteFilePath(const TCHAR * relativePath);
	void openSelectFile();
	void setFileExtFilter(FileDialog & fDlg);
};

class FileRelocalizerDlg : public StaticDialog
{
public :
	FileRelocalizerDlg() = default;
	void init(HINSTANCE hInst, HWND parent) {
		Window::init(hInst, parent);
	};

	int doDialog(const TCHAR *fn, bool isRTL = false);

    virtual void destroy() {
    };

	generic_string getFullFilePath() {
		return _fullFilePath;
	};

protected :
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private :
	generic_string _fullFilePath;

};
