/* ============================================================
* Crusta - Qt5 webengine browser
* Copyright (C) 2017-2018 Anmol Gautam <tarptaeya@gmail.com>
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
#include "tab.h"
#include "bookmarksbar.h"
#include "infobar.h"
#include "toolbar.h"
#include "tabwidget.h"
#include "webview.h"
#include "appmanager.h"

Tab::Tab(QWidget *parent)
    : QWidget(parent)
{
    m_vBoxLayout = new QVBoxLayout(this);
    m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_vBoxLayout->setSpacing(0);
    setLayout(m_vBoxLayout);

    m_toolBar = new ToolBar(this);
    m_vBoxLayout->addWidget(m_toolBar, 0);
}

QString Tab::title() const
{
    if (!m_webView) {
        return QString();
    }
    return m_webView->title();
}

QString Tab::urlString() const
{
    if (!m_webView) {
        return QString();
    }
    return QString::fromUtf8(m_webView->url().toEncoded());
}

QIcon Tab::icon() const
{
    if (!m_webView) {
        return QIcon();
    }
    return m_webView->icon();
}

WebView *Tab::webView()
{
    return m_webView;
}

void Tab::setWebView(WebView *webView)
{
    if (m_webView) {
        return;
    }
    m_webView = webView;
    m_vBoxLayout->addWidget(m_webView, 1);

    connect(m_webView, &WebView::titleChanged, this, [this](const QString &title){
        TabWidget *tabWidget = appManager->getTabWidget(this);
        tabWidget->setTabText(tabWidget->indexOf(this), title);
    });

    connect(m_webView, &WebView::iconChanged, this, [this](const QIcon &icon){
        TabWidget *tabWidget = appManager->getTabWidget(this);
        tabWidget->setTabIcon(tabWidget->indexOf(this), icon);
    });
}
