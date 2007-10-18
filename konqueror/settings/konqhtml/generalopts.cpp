////
// Add here all general options - those that apply to both web browsing and filemanagement mode
//
// (c) Sven Radej 1998
// (c) David Faure 1998
// (c) 2001 Waldo Bastian <bastian@kde.org>
// (c) 2007 Nick Shaforostoff <shafff@ukr.net>

// Own
#include "generalopts.h"

// Qt
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtGui/QGroupBox>
#include <QtGui/QLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

// KDE
#include <konq_defaults.h> // include default values directly from konqueror
#include <kapplication.h>
#include <kurlrequester.h>

// Local
#include "ui_advancedTabOptions.h"
#include "khtml_settings.h"
#include <KPluginFactory>
#include <KPluginLoader>

K_PLUGIN_FACTORY_DECLARATION(KcmKonqHtmlFactory)

//-----------------------------------------------------------------------------

KKonqGeneralOptions::KKonqGeneralOptions(QWidget *parent, const QVariantList&)
    : KCModule( KcmKonqHtmlFactory::componentData(), parent )
{
    m_pConfig = KSharedConfig::openConfig("konquerorrc", KConfig::NoGlobals);
    QGridLayout *lay = new QGridLayout(this);

    //HOME URL
    QHBoxLayout *homeLayout = new QHBoxLayout;

    QLabel *label = new QLabel(i18n("Home &URL:"), this);
    homeLayout->addWidget(label);

    homeURL = new KUrlRequester(this);
    homeURL->setMode(KFile::Directory);
    homeURL->setWindowTitle(i18n("Select Home Folder"));
    homeLayout->addWidget(homeURL);
    connect(homeURL, SIGNAL(textChanged(const QString &)), SLOT(slotChanged()));
    label->setBuddy(homeURL);

    lay->addLayout(homeLayout, 0, 0);

    QString homestr = i18n("This is the URL (e.g. a folder or a web page) where "
                           "Konqueror will jump to when the \"Home\" button is pressed. "
                           "This is usually your home folder, symbolized by a 'tilde' (~).");
    label->setWhatsThis(homestr);
    homeURL->setWhatsThis(homestr);
    //HOME URL

    QGroupBox* tabsGroup = new QGroupBox(i18n("Tabbed Browsing"));

    tabOptions = new Ui_advancedTabOptions;
    tabOptions->setupUi(tabsGroup);

    connect(tabOptions->m_pShowMMBInTabs, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(tabOptions->m_pDynamicTabbarHide, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(tabOptions->m_pNewTabsInBackground, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(tabOptions->m_pOpenAfterCurrentPage, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(tabOptions->m_pTabConfirm, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(tabOptions->m_pTabCloseActivatePrevious, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(tabOptions->m_pPermanentCloseButton, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(tabOptions->m_pKonquerorTabforExternalURL, SIGNAL(clicked()), SLOT(slotChanged()));
    connect(tabOptions->m_pPopupsWithinTabs, SIGNAL(clicked()), SLOT(slotChanged()));

    lay->addWidget( tabsGroup, 1, 0, 1, 2 );

    lay->setRowStretch(1, 1);

    load();
    emit changed(false);
}

KKonqGeneralOptions::~KKonqGeneralOptions()
{
    delete tabOptions;
}

void KKonqGeneralOptions::load()
{
    KConfigGroup cg(m_pConfig, "FMSettings");
    
    homeURL->setUrl(cg.readEntry("HomeURL", "~"));
    tabOptions->m_pShowMMBInTabs->setChecked( cg.readEntry( "MMBOpensTab", false ) );
    tabOptions->m_pDynamicTabbarHide->setChecked( ! (cg.readEntry( "AlwaysTabbedMode", false )) );

    tabOptions->m_pNewTabsInBackground->setChecked( ! (cg.readEntry( "NewTabsInFront", false)) );
    tabOptions->m_pOpenAfterCurrentPage->setChecked( cg.readEntry( "OpenAfterCurrentPage", false) );
    tabOptions->m_pPermanentCloseButton->setChecked( cg.readEntry( "PermanentCloseButton", false) );
    tabOptions->m_pKonquerorTabforExternalURL->setChecked( cg.readEntry( "KonquerorTabforExternalURL", false) );
    tabOptions->m_pPopupsWithinTabs->setChecked( cg.readEntry( "PopupsWithinTabs", false) );
    tabOptions->m_pTabCloseActivatePrevious->setChecked( cg.readEntry( "TabCloseActivatePrevious", false) );

    cg = KConfigGroup(m_pConfig, "Notification Messages");
    tabOptions->m_pTabConfirm->setChecked( !cg.hasKey("MultipleTabConfirm") );

}

void KKonqGeneralOptions::defaults()
{
    homeURL->setUrl(KUrl("~"));

    bool old = m_pConfig->readDefaults();
    m_pConfig->setReadDefaults(true);
    load();
    m_pConfig->setReadDefaults(old);
}

void KKonqGeneralOptions::save()
{
    KConfigGroup cg(m_pConfig, "FMSettings");
    cg.writeEntry( "HomeURL", homeURL->url().isEmpty()? QString("~") : homeURL->url().url() );
    cg.writeEntry( "MMBOpensTab", tabOptions->m_pShowMMBInTabs->isChecked() );
    cg.writeEntry( "AlwaysTabbedMode", !(tabOptions->m_pDynamicTabbarHide->isChecked()) );

    cg.writeEntry( "NewTabsInFront", !(tabOptions->m_pNewTabsInBackground->isChecked()) );
    cg.writeEntry( "OpenAfterCurrentPage", tabOptions->m_pOpenAfterCurrentPage->isChecked() );
    cg.writeEntry( "PermanentCloseButton", tabOptions->m_pPermanentCloseButton->isChecked() );
    cg.writeEntry( "KonquerorTabforExternalURL", tabOptions->m_pKonquerorTabforExternalURL->isChecked() );
    cg.writeEntry( "PopupsWithinTabs", tabOptions->m_pPopupsWithinTabs->isChecked() );
    cg.writeEntry( "TabCloseActivatePrevious", tabOptions->m_pTabCloseActivatePrevious->isChecked() );

    cg.sync();

    // It only matters wether the key is present, its value has no meaning
    cg = KConfigGroup(m_pConfig,"Notification Messages");
    if ( tabOptions->m_pTabConfirm->isChecked() )
        cg.deleteEntry( "MultipleTabConfirm" );
    else
        cg.writeEntry( "MultipleTabConfirm", true );

    // Send signal to all konqueror instances
    QDBusMessage message =
        QDBusMessage::createSignal("/KonqMain", "org.kde.Konqueror.Main", "reparseConfiguration");
    QDBusConnection::sessionBus().send(message);

    emit changed(false);
}


void KKonqGeneralOptions::slotChanged()
{
    emit changed(true);
}

#include "generalopts.moc"

