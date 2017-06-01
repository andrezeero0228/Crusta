/* ============================================================
* Crusta - Qt5 webengine browser
* Copyright (C) 2017 Anmol Gautam <tarptaeya@gmail.com>
*
* THIS FILE IS A PART OF CRUSTA
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */

#include "privatemainview.h"

#include "privatetabwindow.h"
#include "presentationmodenotifier.h"
#include "jseditor.h"
#include "themeeditor.h"
#include "bookmarkmanager.h"
#include "siteinfo.h"

#include <QObject>
#include <QPoint>
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QPushButton>
#include <QAction>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QDir>
#include <QStatusBar>
#include <QWebEngineDownloadItem>
#include <QPrinter>
#include <QPageSetupDialog>

#include <iostream>




void PrivateMainView::closeTab(int index){
    if(this->tabWindow->count()==1)
        PrivateMainView::quit();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    QAction* hist=new QAction();
    hist->setText(webview->title());
    hist->setIcon(webview->icon());
    QUrl u=webview->url();
    webview->page()->deleteLater();
    this->tabWindow->removeTab(index);
    PrivateMainView::addNewTabButton();
    this->recently_closed->addAction(hist);
    connect(hist,&QAction::triggered,this,[this,u]{restoreTab(u);});
}

void PrivateMainView::zoomIn(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->setZoomFactor(webview->zoomFactor()+.20);
}

void PrivateMainView::zoomOut(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->setZoomFactor(webview->zoomFactor()-.20);
}

void PrivateMainView::resetZoom(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->setZoomFactor(1);
}

void PrivateMainView::fullScreen(){
    if(this->window->isFullScreen()){
        this->window->showMaximized();
        this->fullscreen_action->setText(tr("&Show Full Screen"));
    }
    else{
        this->window->showFullScreen();
        this->fullscreen_action->setText(tr("&Exit Full Screen"));
    }
}


void PrivateMainView::tabBarContext(QPoint point){
    if(this->tabWindow->tabBar()->tabAt(point)!=-1){
        int index=this->tabWindow->tabBar()->tabAt(point);
        QWidget* widget=this->tabWindow->widget(index);
        QLayout* layout=widget->layout();
        QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();

        QMenu* contextMenu=new QMenu();
        QAction* rld_tab=new QAction();
        rld_tab=contextMenu->addAction(tr("&Reload Tab"));
        connect(rld_tab,&QAction::triggered,webview,&QWebEngineView::reload);
        contextMenu->addAction(tr("&Mute Tab"));
        contextMenu->addAction(tr("&Pin Tab"));
        QAction* duplicate=new QAction();
        duplicate=contextMenu->addAction(tr("&Duplicate Tab"));
        connect(duplicate,&QAction::triggered,this,[this,webview]{duplicateTab(webview);});
        contextMenu->addAction(tr("&Open in new Window"));
        contextMenu->addAction(tr("&Bookmark Tab"));
        contextMenu->addAction(tr("&Bookmark All Tabs"));
        QAction* closeoth=new QAction();
        closeoth=contextMenu->addAction(tr("&Close Other Tabs"));
        connect(closeoth,&QAction::triggered,this,[this,index]{closeOtherTabs(index);});
        contextMenu->exec(this->tabWindow->tabBar()->mapToGlobal(point));
    }
    else{
        QMenu* barContext=new QMenu();
        QAction* ntab_bar=new QAction();
        ntab_bar=barContext->addAction(tr("&New Tab"));
        connect(ntab_bar,&QAction::triggered,this,&PrivateMainView::addNormalTab);
        QAction* rlod=new QAction();
        rlod=barContext->addAction(tr("&Reload All Tabs"));
        connect(rlod,&QAction::triggered,this,&PrivateMainView::reloadAllTabs);
        barContext->addAction(tr("&Bookmark All Tabs"));
        barContext->addAction(tr("Mute All Tabs"));
        barContext->addAction(tr("&Restore All Tabs"));
        barContext->exec(this->tabWindow->tabBar()->mapToGlobal(point));
    }
}

void PrivateMainView::FindText(){
    try{
        int index=this->tabWindow->currentIndex();
        QWidget* widget=this->tabWindow->widget(index);
        QLayout* layout=widget->layout();
        QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
        this->hbox=new QHBoxLayout();
        this->findwidget->setParent(webview);
        if(findwidget->layout()==NULL){
            this->findwidget->setLayout(this->hbox);
            this->hbox->addWidget(this->close_findwidget);
            this->hbox->addWidget(this->label);
            this->hbox->addWidget(this->text);
            this->hbox->addWidget(this->match_case_btn);
            this->hbox->addWidget(new QLabel());
            this->close_findwidget->setFlat(true);
            this->close_findwidget->setIcon(QIcon(":/res/drawables/close.svg"));
            this->close_findwidget->setFixedWidth(50);
            connect(this->close_findwidget,&QPushButton::clicked,this,&PrivateMainView::hideFindWidget);
            this->label->setText(QString(tr("Search")));
            this->label->setFixedWidth(75);
            this->text->setFixedWidth(380);
            connect(this->text,&QLineEdit::returnPressed,this,&PrivateMainView::findFindWidget);
            connect(this->match_case_btn,&QCheckBox::toggled,this,&PrivateMainView::findFindWidget);
            this->match_case_btn->setText(tr("&Match &Case"));
            this->hbox->setAlignment(Qt::AlignLeft);
            this->findwidget->setFixedHeight(50);
            this->findwidget->setFixedWidth(webview->geometry().width());
            this->findwidget->setObjectName("findwidget");
            this->findwidget->setStyleSheet("#findwidget{border-top:1px solid black;background-color:#ffffff}");
        }
        this->findwidget->show();
        QPropertyAnimation *animation = new QPropertyAnimation(this->findwidget, "pos");
        animation->setDuration(600);
        this->start_findwidget=webview->geometry().height();
        animation->setStartValue(QPoint(0,this->start_findwidget));
        animation->setEndValue(QPoint(0,this->start_findwidget-50));
        animation->start();
        this->text->setFocus();
    }
    catch(...){
        return;
    }
}

void PrivateMainView::findFindWidget(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    QString data=this->text->text();
    if(this->match_case_btn->isChecked()){
        webview->findText("");
        QWebEnginePage::FindFlags options={QWebEnginePage::FindCaseSensitively};
        webview->findText(data,options);
    }
    else{
        webview->findText("");
        webview->findText(data);
    }
}

void PrivateMainView::hideFindWidget(){
    QPropertyAnimation *animation = new QPropertyAnimation(this->findwidget, "pos");
    animation->setDuration(800);
    animation->setStartValue(QPoint(0,this->start_findwidget-50));
    animation->setEndValue(QPoint(0,this->start_findwidget));
    animation->start();
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->findText("");
    this->findwidget->setParent(0);
    this->text->setText("");
}

void PrivateMainView::selectAllText(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->triggerPageAction(QWebEnginePage::SelectAll);
}

void PrivateMainView::enterPresentationMode(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    QString qurl=webview->url().toString();
    QWebEngineView* newwebview=new QWebEngineView();
    newwebview->load(QUrl(qurl));
    newwebview->showFullScreen();
    QAction* newExitAction=new QAction();
    newExitAction->setShortcut(Qt::Key_Escape);
    newwebview->addAction(newExitAction);
    this->p_notifier->setViewParent(newwebview);
    this->p_notifier->showNotifier();
    connect(newExitAction,&QAction::triggered,newwebview,&QWebEngineView::close);
}

void PrivateMainView::undoPageAction(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->triggerPageAction(QWebEnginePage::Undo);
}

void PrivateMainView::redoPageAction(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->triggerPageAction(QWebEnginePage::Redo);
}

void PrivateMainView::cutPageAction(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->triggerPageAction(QWebEnginePage::Cut);
}

void PrivateMainView::copyPageAction(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->triggerPageAction(QWebEnginePage::Copy);
}

void PrivateMainView::pastePageAction(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->triggerPageAction(QWebEnginePage::Paste);
}

PrivateMainView::PrivateMainView(){

    this->window->pview=this;

    QFile f("preference.txt");
    if(!f.exists()){
        f.open(QIODevice::WriteOnly);
        QTextStream in(&f);
        in<<"Search String>>>>>\nUA String>>>>>\nHome Page>>>>>\n";
        f.close();
    }

    this->box->setContentsMargins(0,0,0,0);
    this->tabWindow->tabBar()->setDocumentMode(true);
    this->tabWindow->setElideMode(Qt::ElideRight);
    this->tabWindow->tabBar()->setExpanding(false);
    this->tabWindow->tabBar()->setShape(QTabBar::RoundedNorth);
    this->tabWindow->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this->tabWindow->tabBar(),&QTabBar::tabCloseRequested,this,&PrivateMainView::closeTab);
    connect(this->tabWindow->tabBar(),&QTabBar::customContextMenuRequested,this,&PrivateMainView::tabBarContext);
    createView();
    createMenuBar();
    createTabWindow();
    addNormalTab();

    addNewTabButton();
    this->tabWindow->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    connect(this->tabWindow->tabBar(),&QTabBar::tabBarDoubleClicked,this,&PrivateMainView::tabAreaDoubleClicked);
    connect(this->tabWindow->tabBar(),&QTabBar::tabMoved,this,&PrivateMainView::addNewTabButton);
    connect(this->tabWindow,&QTabWidget::currentChanged,this,&PrivateMainView::addNewTabButton);
    connect(this->newtabbtn,&QPushButton::clicked,this,&PrivateMainView::addNormalTab);
    connect(this->tabWindow,&QTabWidget::currentChanged,this,&PrivateMainView::changeSpinner);

    this->tabWindow->setStyleSheet("QTabWidget::tab-bar{left:0px;height:32} QTabBar{background-color:black;} QTabBar::tab:selected{background-color:white;color:black;max-width:175px;min-width:175px;height:32px} QTabBar::tab:!selected{max-width:173px;min-width:173px;color:black;background-color:#dbdbdb;top:2px;border:0.5px solid black;height:30px} QPushButton{background-color:#dbdbdb;} QPushButton:hover{background-color:white;}");

}

void PrivateMainView::createView(){
    this->window->setWindowTitle("Crusta - Private Mode");
    this->window->setLayout(box);
}

void PrivateMainView::showView(){
    this->window->showMaximized();
}

void PrivateMainView::newWindow(){
    PrivateMainView* newView=new PrivateMainView();
    newView->showView();
}

void PrivateMainView::createMenuBar(){
    this->file_menu=this->menu->addMenu(tr("&File"));
    this->new_tab_action=this->file_menu->addAction(tr("&New Tab"));
    connect(this->new_tab_action,&QAction::triggered,this,&PrivateMainView::addNormalTab);
    this->split_mode_action=this->file_menu->addAction(tr("&Split Tab Mode"));
    connect(this->split_mode_action,&QAction::triggered,this,&PrivateMainView::splitModefx);
    this->file_menu->addSeparator();
    this->open_file=this->file_menu->addAction(tr("&Open File"));
    connect(this->open_file,&QAction::triggered,this,&PrivateMainView::openLocalFile);
    this->save_as_pdf=this->file_menu->addAction(tr("&Save Page As PDF"));
    connect(this->save_as_pdf,&QAction::triggered,this,&PrivateMainView::saveAsPdf);
    this->save_page=this->file_menu->addAction(tr("&Save Page"));
    connect(this->save_page,&QAction::triggered,this,&PrivateMainView::savePage);
    this->capture_screenshot=this->file_menu->addAction(tr("&Capture ScreenShot"));
    this->capture_screenshot->setShortcut(QKeySequence(QKeySequence::Save));
    connect(this->capture_screenshot,&QAction::triggered,this,&PrivateMainView::screenShot);
    this->exit_action=this->file_menu->addAction(tr("&Quit"));
    connect(this->exit_action,&QAction::triggered,this,&PrivateMainView::closeWindow);
    this->edit_menu=this->menu->addMenu(tr("&Edit"));
    this->undo_action=this->edit_menu->addAction(tr("&Undo"));
    connect(this->undo_action,&QAction::triggered,this,&PrivateMainView::undoPageAction);
    this->redo_action=this->edit_menu->addAction(tr("&Redo"));
    connect(this->redo_action,&QAction::triggered,this,&PrivateMainView::redoPageAction);
    this->edit_menu->addSeparator();
    this->cut_action=this->edit_menu->addAction(tr("&Cut"));
    connect(this->cut_action,&QAction::triggered,this,&PrivateMainView::cutPageAction);
    this->copy_action=this->edit_menu->addAction(tr("&Copy"));
    connect(this->copy_action,&QAction::triggered,this,&PrivateMainView::copyPageAction);
    this->paste_action=this->edit_menu->addAction(tr("&Paste"));
    connect(this->paste_action,&QAction::triggered,this,&PrivateMainView::pastePageAction);
    this->edit_menu->addSeparator();
    this->selectall_action=this->edit_menu->addAction(tr("&Select All"));
    connect(this->selectall_action,&QAction::triggered,this,&PrivateMainView::selectAllText);
    this->find_action=this->edit_menu->addAction(tr("&Find"));
    this->find_action->setShortcut(QKeySequence(QKeySequence::Find));
    connect(this->find_action,&QAction::triggered,this,&PrivateMainView::FindText);
    this->preference=this->edit_menu->addAction(tr("&Edit Preference"));
    connect(this->preference,&QAction::triggered,this,&PrivateMainView::editPreference);
    this->view_menu=this->menu->addMenu(tr("&View"));
    this->view_page_source_action=this->view_menu->addAction(tr("&Page Source"));
    connect(this->view_page_source_action,&QAction::triggered,this,&PrivateMainView::viewPageSource);
    this->zoom_in_action=this->view_menu->addAction(tr("&Zoom In"));
    connect(this->zoom_in_action,&QAction::triggered,this,&PrivateMainView::zoomIn);
    this->zoom_out_action=this->view_menu->addAction(tr("&Zoom Out"));
    connect(this->zoom_out_action,&QAction::triggered,this,&PrivateMainView::zoomOut);
    this->reset_zoom_action=this->view_menu->addAction(tr("&Reset Zoom"));
    connect(this->reset_zoom_action,&QAction::triggered,this,&PrivateMainView::resetZoom);
    this->view_menu->addSeparator();
    this->presentation_action=this->view_menu->addAction(tr("&Presentation Mode"));
    connect(this->presentation_action,&QAction::triggered,this,&PrivateMainView::enterPresentationMode);
    this->fullscreen_action=this->view_menu->addAction(tr("&Show Full Screen"));
    connect(this->fullscreen_action,&QAction::triggered,this,&PrivateMainView::fullScreen);
    this->view_menu->addAction(tr("&Reset View"));
    this->history_menu=this->menu->addMenu(tr("&History"));
    this->restore_session=this->history_menu->addAction(tr("Restore Previous Session"));
    this->recently_closed=this->history_menu->addMenu(tr("&Recently Closed"));
    this->bookmark_menu=this->menu->addMenu(tr("&Bookmarks"));
    this->bookmark_tab=this->bookmark_menu->addAction(tr("&Bookmark This Page"));
    connect(this->bookmark_tab,&QAction::triggered,this,&PrivateMainView::bookmarkTab);
    this->bookmark_all_tabs=this->bookmark_menu->addAction(tr("&Bookmark All Tabs"));
    connect(this->bookmark_all_tabs,&QAction::triggered,this,&PrivateMainView::bookmarkAllTabs);
    this->show_all_bookmarks=this->bookmark_menu->addAction(tr("&Show All Bookmarks"));
    connect(this->show_all_bookmarks,&QAction::triggered,this,&PrivateMainView::showBookamrks);
    this->download_menu=this->menu->addMenu(tr("&Downloads"));
    this->show_all_downloads=this->download_menu->addAction(tr("&Download Manager"));
    connect(this->show_all_downloads,&QAction::triggered,this,&PrivateMainView::showDownloads);
    this->download_menu->addAction(tr("&Clear all Downloads"));
    this->tool_menu=this->menu->addMenu(tr("&Tools"));
    this->sitei=this->tool_menu->addAction(tr("&Site Info"));
    connect(this->sitei,&QAction::triggered,this,&PrivateMainView::showPageInfo);
    this->tool_menu->addAction(tr("&Crusta Speak"));
    this->tool_menu->addSeparator();
    this->tool_menu->addAction(tr("&Cookies Manager"));
    this->web_inspector_action=this->tool_menu->addAction(tr("&Web Inspector"));
    this->tool_menu->addSeparator();
    this->devTools=this->tool_menu->addMenu(tr("&Developer Tools"));
    this->runJsCode=this->devTools->addAction(tr("&Run Javascript Code"));
    this->viewSource=this->devTools->addAction(tr("View Page Source"));
    this->changUA=this->devTools->addAction(tr("Edit User Agent"));
    connect(this->viewSource,&QAction::triggered,this,&PrivateMainView::viewPageSource);
    connect(this->runJsCode,&QAction::triggered,this,&PrivateMainView::showJsCodeEditor);
    connect(this->changUA,&QAction::triggered,this,&PrivateMainView::changeUAfx);
    this->help_menu=this->menu->addMenu(tr("&Help"));
    this->help_menu->addAction(tr("&Crusta Help"));
    this->help_menu->addAction(tr("&Crusta Tour"));
    this->aboutCr=this->help_menu->addAction(tr("&About Crusta"));
    connect(this->aboutCr,&QAction::triggered,this,&PrivateMainView::about);
    this->help_menu->addSeparator();
    this->help_menu->addAction(tr("&Keyboard Shortcuts"));
    this->help_menu->addAction(tr("&Request Feature"));
    this->help_menu->addAction(tr("&Report Issue"));
    this->help_menu->addSeparator();
    this->help_menu->addAction(tr("&License"));
    this->window->menu=this->menu;
}

void PrivateMainView::createTabWindow(){
    this->tabWindow->setMovable(true);
    this->tabWindow->setTabsClosable(true);
    this->tabWindow->setContentsMargins(0,0,0,0);
    this->box->addWidget(this->tabWindow);
}

void PrivateMainView::addNormalTab(){
    PrivateTabWindow* tab=new PrivateTabWindow();
    tab->menu_btn->setMenu(menu);
    this->tabWindow->addTab(tab->returnTab(),tr("new Tab"));
    this->tabWindow->setCurrentIndex(this->tabWindow->count()-1);
    int cnt=this->tabWindow->count();
    if(cnt==1){
        this->tabWindow->setTabText(0,tr("Connecting..."));
        QWidget* widget=this->tabWindow->widget(0);
        QLayout* layout=widget->layout();
        WebView* webview=(WebView*)layout->itemAt(1)->widget();
        QString home;
        QFile inputFile("preference.txt");
        if (inputFile.open(QIODevice::ReadOnly))
        {
           QTextStream in(&inputFile);
           while (!in.atEnd())
           {
              QString line = in.readLine();
              QStringList data=line.split(">>>>>");
              if(data[0]=="Home Page"){
                  home=data[1];
                  break;
              }
           }
           inputFile.close();
        }
        if(home.isEmpty()){
            QDialog* w=new QDialog();
            QLabel* lbl=new QLabel(tr("Home Page Url"));
            QLineEdit* url=new QLineEdit();
            QHBoxLayout* hbox=new QHBoxLayout();
            hbox->addWidget(lbl);
            hbox->addWidget(url);
            QHBoxLayout* h1box=new QHBoxLayout();
            QPushButton* ok=new QPushButton(tr("Save"));
            h1box->addWidget(new QLabel());
            h1box->addWidget(ok);
            ok->setFixedWidth(100);
            QVBoxLayout* vbox=new QVBoxLayout();
            vbox->addLayout(hbox);
            vbox->addLayout(h1box);
            w->setLayout(vbox);
            w->setFixedWidth(500);
            w->setWindowFlags(Qt::FramelessWindowHint);
            w->setStyleSheet("QWidget{background-color:white;color:black} QLabel{color:black} QLineEdit{color:black;background-color:white} QPushButton{border:0.5px solid black;padding:4px 8px;color:white;background-color:black} QPushButton:hover{background-color:white;color:black}");
            connect(ok,&QPushButton::clicked,w,&QDialog::accept);
            if(w->exec()!=QDialog::Accepted){
                return;
            }
            if(url->text()=="")
                return;
            home=url->text();
            QFile f("preference.txt");
            if(f.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QString s;
                QTextStream t(&f);
                while(!t.atEnd())
                {
                    QString line = t.readLine();
                    QStringList data=line.split(">>>>>");
                    if(data[0]=="Home Page")
                        s.append(data[0]+">>>>>"+home+"\n");
                    else
                        s.append(line+"\n");
                }
                f.resize(0);
                t << s;
                f.close();
            }
        }
        webview->home_page=home;
        webview->load(home);
    }
    PrivateMainView::addNewTabButton();
}

void PrivateMainView::viewPageSource(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    QString qurl=webview->url().toString();
    qurl=QString("view-source:")+qurl;
    PrivateTabWindow* tab=new PrivateTabWindow();
    tab->menu_btn->setMenu(menu);
    index++;
    this->tabWindow->insertTab(index,tab->returnTab(),tr("new Tab"));
    this->tabWindow->setCurrentIndex(index);
    QWidget* wid=this->tabWindow->widget(index);
    QLayout* lay=wid->layout();
    QWebEngineView* webv=(QWebEngineView*)lay->itemAt(1)->widget();
    webv->load(QUrl(qurl));
    PrivateMainView::addNewTabButton();
}

void PrivateMainView::saveAsPdf(){
    QPrinter printer;
    printer.setPageLayout(currentPageLayout);
    QPageSetupDialog dlg(&printer, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    currentPageLayout.setPageSize(printer.pageLayout().pageSize());
    currentPageLayout.setOrientation(printer.pageLayout().orientation());
    QFileDialog f;
    f.setOption(QFileDialog::DontUseNativeDialog,true);
    QString file_name=f.getSaveFileName(this->window,tr("Crusta : Save File"),QDir::homePath(),"Pdf File(*.pdf)",nullptr,f.options());
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    if(file_name!=""){
    if(!file_name.endsWith(".pdf"))file_name+=QString(".pdf");
    webview->page()->printToPdf(file_name,QPageLayout(printer.pageLayout().pageSize(),printer.pageLayout().orientation(),printer.pageLayout().margins()));
    }
}

void PrivateMainView::savePage(){
    QFileDialog f;
    f.setOption(QFileDialog::DontUseNativeDialog,true);
    QString file_name=f.getSaveFileName(this->window,tr("Crusta : Save File"),QDir::homePath(),"WebPage, Complete",nullptr,f.options());
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    if(file_name!=QString("")){
        webview->page()->save(file_name,QWebEngineDownloadItem::CompleteHtmlSaveFormat);
    }
}

void PrivateMainView::showJsCodeEditor(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    jsEditor->setView(webview);
    jsEditor->show();
}

void PrivateMainView::openLocalFile(){
    QFileDialog f;
    f.setOption(QFileDialog::DontUseNativeDialog,true);
    QString filename=f.getOpenFileName(this->window,tr("Crusta : Open File"),QDir::homePath(),QString(),nullptr,f.options());
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    if(filename!=""){
    webview->load(QUrl(QString("file://")+filename));
    }
}

void PrivateMainView::screenShot(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    QPixmap pmap = webview->grab();
    QFileDialog f;
    f.setOption(QFileDialog::DontUseNativeDialog,true);
    QString filename=f.getSaveFileName(this->window,tr("Crusta : Open File"),QDir::homePath(),QString("Images (*.png *.xpm *.jpg *.bmp)"),nullptr,f.options());
    if(filename!=""){
    if(!(filename.endsWith(".png")||filename.endsWith(".jpg")||filename.endsWith(".bmp")||filename.endsWith(".xpm")))filename+=QString(".png");
    pmap.save(filename);
    }
}

void PrivateMainView::tabAreaDoubleClicked(int index){
    if(index==-1){
        PrivateMainView::addNormalTab();
    }
}

void PrivateMainView::addNewTabButton(){
    int cnt=this->tabWindow->count();
    int x=cnt*175+5; //size of a tab;
    if(newtabbtn->parent()==NULL)newtabbtn->setParent(this->tabWindow->tabBar());
    this->newtabbtn->move(x,5);
    this->newtabbtn->setFlat(false);
    this->newtabbtn->setFixedHeight(22);
    this->newtabbtn->setFixedWidth(25);
}

void PrivateMainView::editPreference(){
    ThemeEditor* th=new ThemeEditor();
    th->setWindowTitle(tr("Theme Editor"));
    th->show();
}

void PrivateMainView::duplicateTab(QWebEngineView* view){
    TabWindow* tab=new TabWindow();
    tab->menu_btn->setMenu(menu);
    WebView* wview=new WebView();
    wview->load(view->url());
    this->tabWindow->addTab(tab->returnTab(wview),tr("new Tab"));
    this->tabWindow->setCurrentIndex(this->tabWindow->count()-1);
    PrivateMainView::addNewTabButton();
}

void PrivateMainView::reloadAllTabs(){
    int cnt=this->tabWindow->count();
    for(int i=0;i<cnt;i++){
        QWidget* widget=this->tabWindow->widget(i);
        QLayout* layout=widget->layout();
        QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
        webview->reload();
    }
}

void PrivateMainView::closeOtherTabs(int index){
    int cnt=this->tabWindow->count();
    int i=0;
    while(i<cnt){
        if(i!=index){
            QWidget* widget=this->tabWindow->widget(i);
            QLayout* layout=widget->layout();
            QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
            webview->page()->windowCloseRequested();
            if(index>0)index--;
            cnt--;
            i=0;
            PrivateMainView::addNewTabButton();
            continue;
        }
        i++;
    }
}

void PrivateMainView::restoreTab(QUrl u){
    PrivateMainView::addNormalTab();
    int index=this->tabWindow->count()-1;
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->load(u);
}

void PrivateMainView::about(){
    PrivateMainView::addNormalTab();
    int index=this->tabWindow->count()-1;
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    webview->load(QUrl("http://www.crustabrowser.com/about"));
}

void PrivateMainView::showBookamrks(){
    PrivateBookmarkManager* b=new PrivateBookmarkManager(this);
    b->show();
}

void PrivateMainView::bookmarkTab(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    WebView* webview=(WebView*)layout->itemAt(1)->widget();

    QFile file("bookmarks.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream out(&file);
    out << webview->title().toLatin1()+">>>>>"+webview->url().toString().toLatin1()+">>>>>"+"\n";
    file.close();
}

void PrivateMainView::bookmarkAllTabs(){
    int cnt=this->tabWindow->count();
    for(int index=0;index<cnt;index++){
        QWidget* widget=this->tabWindow->widget(index);
        QLayout* layout=widget->layout();
        WebView* webview=(WebView*)layout->itemAt(1)->widget();

        QFile file("bookmarks.txt");
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream out(&file);
        out << webview->title().toLatin1()+">>>>>"+webview->url().toString().toLatin1()+">>>>>"+"\n";
        file.close();
    }
}

void PrivateMainView::quit(){
    this->window->deleteLater();
}

void PrivateMainView::showPageInfo(){
    int index=this->tabWindow->currentIndex();
    QWidget* widget=this->tabWindow->widget(index);
    QLayout* layout=widget->layout();
    QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
    SiteInfoWidget* sw=new SiteInfoWidget(webview);
    sw->exec();
}

void PrivateMainView::changeSpinner(int index){
    int cnt=this->tabWindow->count();
    for(int i=0;i<cnt;i++){
        QWidget* widget=this->tabWindow->widget(i);
        QLayout* layout=widget->layout();
        QWebEngineView* webview=(QWebEngineView*)layout->itemAt(1)->widget();
        if(!webview->icon().isNull())
            continue;
        QLabel* lbl=new QLabel();
        if(i!=index){
            QMovie* mov=new QMovie(":/res/videos/passive_loader.gif");
            lbl->setMovie(mov);
            mov->start();
        }
        else{
            QMovie* mov=new QMovie(":/res/videos/loader.gif");
            lbl->setMovie(mov);
            mov->start();
        }
        this->tabWindow->tabBar()->setTabButton(i,QTabBar::LeftSide,lbl);
    }
}

void PrivateMainView::showDownloads(){
    this->d_manager=this->window->d_manager;
    this->d_manager->show();
}

void PrivateMainView::changeUAfx(){
    PrivateAddressLineEdit* pad=new PrivateAddressLineEdit();
    pad->setUAString();
    pad->deleteLater();
}

void PrivateMainView::splitModefx(){
    if(this->split_mode_action->text()==QString(tr("&Exit Split Mode"))){
        if(this->box->count()==2){
            this->box->itemAt(1)->widget()->deleteLater();
            this->box->removeItem(this->box->itemAt(1));
            this->split_mode_action->setText(tr("&Split Mode"));
            return;
        }
        else{
            PWindow* p=(PWindow*)this->window->parent();
            p->layout()->itemAt(1)->widget()->deleteLater();
            p->layout()->removeItem(p->layout()->itemAt(1));
            return;
        }
    }
    box->setContentsMargins(0,0,0,0);
    box->setSpacing(0);
    PrivateMainView* psplitView=new PrivateMainView();
    box->addWidget(psplitView->window);
    psplitView->split_mode_action->setText(tr("&Exit Split Mode"));
    this->split_mode_action->setText(tr("&Exit Split Mode"));
}

void PrivateMainView::closeWindow(){
    if(this->split_mode_action->text()==QString(tr("&Exit Split Mode"))){
        if(this->box->count()==2){
            this->box->itemAt(1)->widget()->deleteLater();
            this->box->removeItem(this->box->itemAt(1));
            this->split_mode_action->setText(tr("&Split Mode"));
            return;
        }
        else{
            PWindow* p=(PWindow*)this->window->parent();
            p->layout()->itemAt(1)->widget()->deleteLater();
            p->layout()->removeItem(p->layout()->itemAt(1));
            return;
        }
    }
    int cnt=this->tabWindow->count();
    for(int i=0;i<cnt;i++){
        this->closeTab(0);
    }
}

void PWindow::closeEvent(QCloseEvent* event){
    this->pview->closeWindow();
    event->accept();
}
