/*
 * Copyright (C) Kreogist Dev Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <QBoxLayout>
#include <QSignalMapper>

//Music Global.
#include "knmusicglobal.h"

//Ports
#include "knmusicbackend.h"
#include "knmusicparser.h"
#include "knmusicsearchbase.h"
#include "knmusicsolomenubase.h"
#include "knmusicdetaildialogbase.h"
#include "knmusicdetailtooltipbase.h"
#include "knmusicheaderplayerbase.h"
#include "knmusicheaderlyricsbase.h"
#include "knmusicnowplayingbase.h"
#include "knmusiclibrarybase.h"
#include "knmusicplaylistmanagerbase.h"
#include "knmusicmainplayerbase.h"

//Plugins
#ifdef ENABLE_LIBBASS
#include "plugin/knmusicbackendbass/knmusicbackendbass.h"
#include "plugin/knmusicbackendbass/knmusicbassanalysiser.h"
#endif
#ifdef ENABLE_FFMPEG
#include "plugin/knmusicffmpeganalysiser/knmusicffmpeganalysiser.h"
#endif
#ifdef ENABLE_LIBVLC
#include "plugin/knmusicbackendvlc/knmusicbackendvlc.h"
#endif
#include "plugin/knmusictagid3v1/knmusictagid3v1.h"
#include "plugin/knmusictagflac/knmusictagflac.h"
#include "plugin/knmusictagid3v2/knmusictagid3v2.h"
#include "plugin/knmusictagm4a/knmusictagm4a.h"
#include "plugin/knmusictagwma/knmusictagwma.h"
#include "plugin/knmusictagapev2/knmusictagapev2.h"
#include "plugin/knmusictagid3v2/knmusictagwav.h"
#include "plugin/knmusicdetaildialog/knmusicdetaildialog.h"
#include "plugin/knmusicdetailtooltip/knmusicdetailtooltip.h"
#include "plugin/knmusicsearch/knmusicsearch.h"
#include "plugin/knmusiccueparser/knmusiccueparser.h"
#include "plugin/knmusicheaderplayer/knmusicheaderplayer.h"
#include "plugin/knmusicsolomenu/knmusicsolomenu.h"
#include "plugin/knmusicmultimenu/knmusicmultimenu.h"
#include "plugin/knmusicheaderlyrics/knmusicheaderlyrics.h"
#include "plugin/knmusicnowplaying/knmusicnowplaying.h"
#include "plugin/knmusiclibrary/knmusiclibrary.h"
#include "plugin/knmusicplaylistmanager/knmusicplaylistmanager.h"
#include "plugin/knmusicmainplayer/knmusicmainplayer.h"

#include "knglobal.h"
#include "knmusictab.h"
#include "knmousedetectheader.h"
#include "knplatformextras.h"
#include "knconnectionhandler.h"
#include "knmusiccategorytabwidget.h"
#include "knlocalemanager.h"
#include "knpreferencewidgetspanel.h"

#include "knmusicplugin.h"

#include <QDebug>

KNMusicPlugin::KNMusicPlugin(QObject *parent) :
    KNAbstractMusicPlugin(parent)
{
    //Initial infrastructure.
    initialInfrastructure();
    //Load detail info first.
    loadDetailInfo(new KNMusicDetailDialog);
    //Initial parser.
    initialParser();
    //Initial menus.
    initialSoloMenu(new KNMusicSoloMenu);
    initialMultiMenu(new KNMusicMultiMenu);

    //Load plugins.
    loadSearch(new KNMusicSearch);
#ifdef ENABLE_LIBBASS
    loadBackend(new KNMusicBackendBass);
#endif
#ifdef ENABLE_LIBVLC
    loadBackend(new KNMusicBackendVLC);
#endif
    loadDetailTooptip(new KNMusicDetailTooltip);
    loadNowPlaying(new KNMusicNowPlaying);
    loadHeaderPlayer(new KNMusicHeaderPlayer);
    loadHeaderLyrics(new KNMusicHeaderLyrics);
//    loadMainPlayer(new KNMusicMainPlayer);
    loadLibrary(new KNMusicLibrary);
    loadPlaylistManager(new KNMusicPlaylistManager);

    //Connect retranslate request.
    connect(KNLocaleManager::instance(), &KNLocaleManager::requireRetranslate,
            this, &KNMusicPlugin::retranslate);
}

KNMusicPlugin::~KNMusicPlugin()
{
    //Stop threads.
    m_parserThread.quit();
    m_parserThread.wait();
    //Ask to save the configure.
    emit requireSaveConfigure();
    //Delete all the plugins.
    while(!m_pluginList.isEmpty())
    {
        //We only need to remove the plugin which don't have a parent.
        QObject *currentPlugin=m_pluginList.takeFirst();
        if(currentPlugin->parent()==nullptr)
        {
            delete currentPlugin;
        }
    }
}

QString KNMusicPlugin::caption()
{
    return tr("Music");
}

QPixmap KNMusicPlugin::icon()
{
    return QPixmap(":/plugin/music/common/icon.png");
}

QPixmap KNMusicPlugin::headerIcon()
{
    return QPixmap(":/plugin/configure/music/headicon.png");
}

QPixmap KNMusicPlugin::preferenceIcon()
{
    return QPixmap(":/plugin/configure/music/icon.png");
}

QWidget *KNMusicPlugin::centralWidget()
{
    return m_centralWidget;
}

QWidget *KNMusicPlugin::headerWidget()
{
    return m_headerWidget;
}

KNPreferenceWidgetsPanel *KNMusicPlugin::preferencePanelWidget()
{
    return m_preferencePanel;
}

inline void KNMusicPlugin::loadSearch(KNMusicSearchBase *plugin)
{
    //Give the central widget as the default focus source.
    plugin->setDefaultFocusSource(m_centralWidget);
    //Add plugin to the list.
    m_pluginList.append(plugin);
    //Link global search focus.
    KNMusicGlobal::setMusicSearch(plugin);
    //Add the searcher box to the right most plugin.
    addRightHeaderWidget(plugin->searchBox());
    //Set the next tab focus to the content widget.
    plugin->searchBox()->setTabOrder(plugin->searchBox(),
                                     m_centralWidget);
}

inline void KNMusicPlugin::loadDetailTooptip(KNMusicDetailTooltipBase *plugin)
{
    //Add plugin to the list.
    m_pluginList.append(plugin);
    //Link global detail tooltip widget.
    KNMusicGlobal::setDetailTooltip(plugin);
    //Set backend.
    plugin->setBackend(m_backend);
}

inline void KNMusicPlugin::loadBackend(KNMusicBackend *plugin)
{
    if(m_backend==nullptr)
    {
        m_backend=plugin;
        //Add plugin to the list.
        m_pluginList.append(m_backend);
    }
}

inline void KNMusicPlugin::loadDetailInfo(KNMusicDetailDialogBase *plugin)
{
    //Add plugin to the list.
    m_pluginList.append(plugin);
    //Linke global detail dialog widget.
    KNMusicGlobal::setDetailDialog(plugin);
}

inline void KNMusicPlugin::loadHeaderPlayer(KNMusicHeaderPlayerBase *plugin)
{
    if(m_headerPlayer==nullptr)
    {
        m_headerPlayer=plugin;
        //Link the save configure signal.
        connect(this, &KNMusicPlugin::requireSaveConfigure,
                m_headerPlayer, &KNMusicHeaderPlayerBase::saveConfigure);
        //Set the backend and now playing controls to the header player.
        m_headerPlayer->setBackend(m_backend);
        m_headerPlayer->setNowPlaying(m_nowPlaying);
        //Restore the preference.
        m_headerPlayer->loadConfigure();
        //Add plugin to the list.
        m_pluginList.append(m_headerPlayer);
        //Link player to sense header.
        connect(m_headerPlayer, &KNMusicHeaderPlayerBase::requireCheckCursor,
                m_headerWidget, &KNMouseDetectHeader::checkCurrentCursorPos);
        connect(m_headerWidget, &KNMouseDetectHeader::requireActivateWidget,
                m_headerPlayer, &KNMusicHeaderPlayerBase::activatePlayer);
        connect(m_headerWidget, &KNMouseDetectHeader::requireInactivateWidget,
                m_headerPlayer, &KNMusicHeaderPlayerBase::inactivatePlayer);
        //Linke the request to category tab widget.
        connect(m_headerPlayer, &KNMusicHeaderPlayerBase::requireShowMainPlayer,
                m_centralWidget, &KNMusicCategoryTabWidget::showMainPlayer);
        //Add to main window.
        addLeftHeaderWidget(m_headerPlayer);
    }
}

void KNMusicPlugin::loadMainPlayer(KNMusicMainPlayerBase *plugin)
{
    if(m_mainPlayer==nullptr)
    {
        m_mainPlayer=plugin;
        //Configure the main player.
        m_mainPlayer->hide();
        //Restore the settings.
        ;
        //Add plugin to the list.
        m_pluginList.append(m_mainPlayer);
        //Set the main player.
        m_centralWidget->setMainPlayer(m_mainPlayer);
    }
}

inline void KNMusicPlugin::loadHeaderLyrics(KNMusicHeaderLyricsBase *plugin)
{
    //Add plugin to the list.
    m_pluginList.append(plugin);
    //Link the display to the header player.
    plugin->setHeaderPlayer(m_headerPlayer);
    //Add widget to the header.
    addLeftHeaderWidget(plugin, 1);
}

inline void KNMusicPlugin::loadNowPlaying(KNMusicNowPlayingBase *plugin)
{
    if(m_nowPlaying==nullptr)
    {
        m_nowPlaying=plugin;
        //Set the header player.
        m_nowPlaying->setBackend(m_backend);
        //Restore configure.
        m_nowPlaying->restoreConfigure();
        //Add plugin to list.
        m_pluginList.append(m_nowPlaying);
        //Set global now playing plugin.
        KNMusicGlobal::setNowPlaying(m_nowPlaying);
    }
}

inline void KNMusicPlugin::loadLibrary(KNMusicLibraryBase *plugin)
{
    m_pluginList.append(plugin);
    //Add tabs.
    addMusicTab(plugin->songTab());
    addMusicTab(plugin->artistTab());
    addMusicTab(plugin->albumTab());
    addMusicTab(plugin->genreTab());
    //Set the player.
    plugin->setHeaderPlayer(m_headerPlayer);
}

inline void KNMusicPlugin::loadPlaylistManager(KNMusicPlaylistManagerBase *plugin)
{
    m_pluginList.append(plugin);
    //Add tabs.
    addMusicTab(plugin->categoryTab());
}

void KNMusicPlugin::onArgumentsAvailable(const QStringList &data)
{
    KNMusicGlobal::nowPlaying()->playTemporaryFiles(data);
}

void KNMusicPlugin::retranslate()
{
    //Update the tab caption.
    for(auto i=m_tabList.begin();
        i!=m_tabList.end();
        ++i)
    {
        m_centralWidget->setTabText((*i).index, (*i).tab->caption());
    }
}

void KNMusicPlugin::enablePlatformExtras()
{
    if(m_platformExtras)
    {
        m_extraHandler->addConnectionHandle(
                    connect(m_platformExtras, &KNPlatformExtras::requirePlayPrev,
                            m_nowPlaying, &KNMusicNowPlayingBase::playPrevious));
        m_extraHandler->addConnectionHandle(
                    connect(m_platformExtras, &KNPlatformExtras::requirePlayNext,
                            m_nowPlaying, &KNMusicNowPlayingBase::playNext));
        m_extraHandler->addConnectionHandle(
                    connect(m_platformExtras, &KNPlatformExtras::requirePlay,
                            m_backend, &KNMusicBackend::play));
        m_extraHandler->addConnectionHandle(
                    connect(m_platformExtras, &KNPlatformExtras::requirePause,
                            m_backend, &KNMusicBackend::pause));
    }
}

void KNMusicPlugin::disablePlatformExtras()
{
    if(m_platformExtras)
    {
        m_extraHandler->disconnectAll();
    }
}

int KNMusicPlugin::addMusicCategory(const QPixmap &icon,
                                    const QString &caption,
                                    QWidget *widget)
{
    return m_centralWidget->addTab(icon, caption, widget);
}

void KNMusicPlugin::addLeftHeaderWidget(QWidget *widget,
                                        int stretch,
                                        Qt::Alignment alignment)
{
    m_headerLeftLayout->addWidget(widget, stretch, alignment);
}

void KNMusicPlugin::addRightHeaderWidget(QWidget *widget,
                                         int stretch,
                                         Qt::Alignment alignment)
{
    m_headerRightLayout->addWidget(widget, stretch, alignment);
}

void KNMusicPlugin::onActionShowTab(const int &tabIndex)
{
    m_centralWidget->setCurrentIndex(tabIndex);
}

inline void KNMusicPlugin::initialInfrastructure()
{
    //Initial the music global.
    m_musicGlobal=KNMusicGlobal::instance();
    m_musicGlobal->setNoAlbumArt(QPixmap(":/plugin/music/common/noalbum.png"));

    //Initial preference panel.
    m_preferencePanel=new KNPreferenceWidgetsPanel;
    m_preferencePanel->setPanelName("Music");
    m_musicGlobal->setPreferencePanel(m_preferencePanel);

    //Initial central widget.
    m_centralWidget=new KNMusicCategoryTabWidget;

    //Initial header widget.
    m_headerWidget=new KNMouseDetectHeader;
    //Set header properties.
    m_headerWidget->setContentsMargins(0,0,0,0);
    //Set header layout.
    QBoxLayout *headerLayout=new QBoxLayout(QBoxLayout::LeftToRight,
                                            m_headerWidget);
    headerLayout->setContentsMargins(0,0,10,0);
    headerLayout->setSpacing(0);
    m_headerWidget->setLayout(headerLayout);
    //Initial left layout.
    m_headerLeftLayout=new QBoxLayout(QBoxLayout::LeftToRight,
                                      headerLayout->widget());
    m_headerLeftLayout->setContentsMargins(0,0,0,0);
    m_headerLeftLayout->setSpacing(0);
    headerLayout->addLayout(m_headerLeftLayout, 1);
    //Initial right layout.
    m_headerRightLayout=new QBoxLayout(QBoxLayout::RightToLeft,
                                       headerLayout->widget());
    m_headerRightLayout->setContentsMargins(10,0,0,0);
    m_headerRightLayout->setSpacing(0);
    headerLayout->addLayout(m_headerRightLayout);

    //Initial the extra platform connection handler.
    m_extraHandler=new KNConnectionHandler(this);

    //Initial the tab switch signal mapper.
    m_tabSwitchMapper=new QSignalMapper(this);
    connect(m_tabSwitchMapper, SIGNAL(mapped(int)),
            this, SLOT(onActionShowTab(int)));
}

inline void KNMusicPlugin::initialParser()
{
    //Initial the music parser.
    KNMusicParser *parser=new KNMusicParser;

    //Install all list parser plugins here.
    parser->installListParser(new KNMusicCueParser);

    //Install all tag parser plugins here.
    parser->installTagParser(new KNMusicTagID3v1);
    parser->installTagParser(new KNMusicTagAPEv2);
    parser->installTagParser(new KNMusicTagFLAC);
    parser->installTagParser(new KNMusicTagID3v2);
    parser->installTagParser(new KNMusicTagM4A);
    parser->installTagParser(new KNMusicTagWMA);
    parser->installTagParser(new KNMusicTagWAV);

    //Install all analysiser plugins here.
#ifdef ENABLE_LIBBASS
    parser->installAnalysiser(new KNMusicBassAnalysiser);
#endif
#ifdef ENABLE_FFMPEG
    parser->installAnalysiser(new KNMusicFFMpegAnalysiser);
#endif

    //Add this to plugin list.
    m_pluginList.append(parser);
    //Move to working thread.
    parser->moveToThread(&m_parserThread);
    //Set the parser.
    KNMusicGlobal::setParser(parser);
}

inline void KNMusicPlugin::initialSoloMenu(KNMusicSoloMenuBase *soloMenu)
{
    //Add this to plugin list.
    m_pluginList.append(soloMenu);
    //Set the solo menu.
    KNMusicGlobal::setSoloMenu(soloMenu);
}

inline void KNMusicPlugin::initialMultiMenu(KNMusicMultiMenuBase *multiMenu)
{
    //Add this to plugin list.
    m_pluginList.append(multiMenu);
    //Set the multi menu.
    KNMusicGlobal::setMultiMenu(multiMenu);
}

inline void KNMusicPlugin::addMusicTab(KNMusicTab *musicTab)
{
    //Just add them to a new music category.
    MusicTabItem currentTab;
    currentTab.index=addMusicCategory(musicTab->icon(),
                                      musicTab->caption(),
                                      musicTab->widget());
    currentTab.tab=musicTab;
    //Connect show tab request.
    connect(musicTab, SIGNAL(requireShowTab()),
            m_tabSwitchMapper, SLOT(map()));
    m_tabSwitchMapper->setMapping(musicTab, m_tabList.size());
    //Add tab to list.
    m_tabList.append(currentTab);
    //Connect request to the music tab.
    connect(KNMusicGlobal::musicSearch(), &KNMusicSearchBase::requireSearch,
            musicTab, &KNMusicTab::onActionSearch);
}

inline void KNMusicPlugin::startThreads()
{
    m_parserThread.start();
}

void KNMusicPlugin::setPlatformExtras(KNPlatformExtras *plugin)
{
    m_platformExtras=plugin;
}
