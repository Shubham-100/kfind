/*
   kmanualproxydlg.cpp - Proxy configuration dialog

   Copyright (C) 2001- Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License (GPL) version 2 as published by the Free Software
   Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>

#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kio/ioslave_defaults.h>

#include "kproxyexceptiondlg.h"
#include "kmanualproxydlg.h"


KManualProxyDlg::KManualProxyDlg( QWidget* parent, const char* name )
                :KProxyDialogBase( parent, name, true,
                                   i18n("Manual Proxy Configuration") )
{
    QWidget *page = new QWidget( this );
    setMainWidget (page);
    QVBoxLayout* mainLayout = new QVBoxLayout( page, KDialog::marginHint(),
                                               KDialog::spacingHint() );
    m_gbHostnames = new QGroupBox( i18n("Servers"), page, "m_gbHostnames" );
    m_gbHostnames->setSizePolicy( QSizePolicy(QSizePolicy::Expanding,
                                           QSizePolicy::Fixed,
                                           m_gbHostnames->sizePolicy().hasHeightForWidth()) );
    m_gbHostnames->setColumnLayout(0, Qt::Vertical );
    m_gbHostnames->layout()->setSpacing( 0 );
    m_gbHostnames->layout()->setMargin( 0 );

    QGridLayout* serversLayout = new QGridLayout( m_gbHostnames->layout() );
    serversLayout->setAlignment( Qt::AlignTop );
    serversLayout->setSpacing( KDialog::spacingHint() );
    serversLayout->setMargin( KDialog::marginHint() );

    QGridLayout* glay = new QGridLayout;
    glay->setSpacing( 6 );
    glay->setMargin( 0 );

    m_cbHttp = new QCheckBox( i18n("&HTTP:"), m_gbHostnames, "m_cbHttp" );
    
    QWhatsThis::add( m_cbHttp, i18n("Check this box to enable manual proxy "
                                        "setup for HTTP connections.") );

    m_leHttp = new KLineEdit( m_gbHostnames, "m_leHttp" );
    m_leHttp->setMinimumWidth( m_leHttp->fontMetrics().width('W') * 20 );
    m_leHttp->setEnabled( false );
    m_leHttp->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding,
                                         QSizePolicy::Fixed,
                                         m_leHttp->sizePolicy().hasHeightForWidth()) );

    QWhatsThis::add( m_leHttp, i18n("Enter the address of the HTTP proxy "
                                        "server.") );
    
    QLabel* label = new QLabel( i18n("Port"), m_gbHostnames, "lbl_httpport");
    connect( m_cbHttp, SIGNAL( toggled(bool) ), label,
             SLOT( setEnabled(bool) ) );
    label->setEnabled( false );
    label->setSizePolicy( QSizePolicy(QSizePolicy::Minimum,
                                      QSizePolicy::Fixed,
                                      label->sizePolicy().hasHeightForWidth()) );
    m_sbHttp = new QSpinBox( m_gbHostnames, "m_sbHttp" );
    m_sbHttp->setEnabled( false );
    
    QWhatsThis::add( m_sbHttp, i18n("Enter the port number of the HTTP "
                                    "proxy server. Default is 8080. "
                                    "Another common value is 3128.") );

    glay->addWidget( m_cbHttp, 0, 0 );
    glay->addWidget( m_leHttp, 0, 1 );
    glay->addWidget( label, 0, 2 );
    glay->addWidget( m_sbHttp, 0, 3 );

    m_cbHttps = new QCheckBox( i18n("HTTP&S:"), m_gbHostnames, "m_cbHttps" );
    
    QWhatsThis::add( m_cbHttps, i18n("Check this box to enable manual "
                                     "proxy setup for secure web "
                                     "connections (HTTPS).") );

    m_leHttps = new KLineEdit( m_gbHostnames, "m_leHttps" );
    m_leHttps->setEnabled( FALSE );
    m_leHttps->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding,
                                            QSizePolicy::Fixed,
                                            m_leHttps->sizePolicy().hasHeightForWidth()) );
    
    QWhatsThis::add( m_leHttps, i18n("Enter the address of the secure "
                                       "proxy server.") );
    
    label = new QLabel( i18n("Port"), m_gbHostnames, "lbl_secureport" );
    connect( m_cbHttps, SIGNAL( toggled(bool) ), label,
             SLOT( setEnabled(bool) ) );
    label->setEnabled( false );
    label->setSizePolicy( QSizePolicy(QSizePolicy::Minimum,
                                      QSizePolicy::Fixed,
                                      label->sizePolicy().hasHeightForWidth()) );
    m_sbHttps = new QSpinBox( m_gbHostnames, "sb_secport" );
    m_sbHttps->setEnabled( false );
    
    QWhatsThis::add( m_sbHttps, i18n("Enter the port number of the secure "
                                     "proxy server. Default is 8080. "
                                     "Another common value is 3128.") );

    glay->addWidget( m_cbHttps, 1, 0 );
    glay->addWidget( m_leHttps, 1, 1 );
    glay->addWidget( label, 1, 2 );
    glay->addWidget( m_sbHttps, 1, 3 );

    m_cbFtp = new QCheckBox( i18n("&FTP:"), m_gbHostnames, "m_cbFtp" );
    
    QWhatsThis::add( m_cbFtp, i18n("Check this box to enable manual proxy "
                                   "setup for FTP connections.") );

    m_leFtp = new KLineEdit( m_gbHostnames, "m_leFtp" );
    m_leFtp->setEnabled( false );
    m_leFtp->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding,
                                            QSizePolicy::Fixed,
                                            m_leFtp->sizePolicy().hasHeightForWidth()) );
    
    QWhatsThis::add( m_leFtp, i18n("Enter the address of the FTP proxy "
                                       "server") );
    
    label = new QLabel( i18n("Port"), m_gbHostnames, "lbl_ftpport" );
    connect( m_cbFtp, SIGNAL( toggled(bool) ), label,
             SLOT( setEnabled(bool) ) );
    label->setEnabled( false );
    label->setSizePolicy( QSizePolicy(QSizePolicy::Minimum,
                                      QSizePolicy::Fixed,
                                      label->sizePolicy().hasHeightForWidth()) );
    m_sbFtp = new QSpinBox( m_gbHostnames, "sb_ftpport" );
    m_sbFtp->setEnabled( false );

    QWhatsThis::add( m_sbFtp, i18n("Enter the port number of the FTP proxy "
                                   "server. Default 8080. Another common value "
                                   "is 3128.") );

    glay->addWidget( m_cbFtp, 2, 0 );
    glay->addWidget( m_leFtp, 2, 1 );
    glay->addWidget( label, 2, 2 );
    glay->addWidget( m_sbFtp, 2, 3 );

    serversLayout->addLayout( glay, 0, 0 );

    QVBoxLayout* vlay = new QVBoxLayout;
    vlay->setSpacing( KDialog::spacingHint() );
    vlay->setMargin( 0 );

    m_pbCopyDown = new QPushButton( m_gbHostnames, "m_pbCopyDown" );
    m_pbCopyDown->setPixmap( BarIcon("down", KIcon::SizeSmall) );
    m_pbCopyDown->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                             QSizePolicy::Fixed,
                                             m_pbCopyDown->sizePolicy().hasHeightForWidth()) );

    QWhatsThis::add( m_pbCopyDown, i18n("<qt>This button allows you to copy "
                                       "the entry of one input field into all "
                                       "the others underneath it. For "
                                       "example, if you fill out the "
                                       "information for <tt>HTTP</tt> and "
                                       "press this button, whatever you "
                                       "entered will be copied to all the "
                                       "fields below that are enabled!") );
    vlay->addWidget( m_pbCopyDown );
    QSpacerItem* spacer = new QSpacerItem( 1, 1 );
    vlay->addItem( spacer );

    serversLayout->addLayout( vlay, 0, 1 );
    mainLayout->addWidget( m_gbHostnames );

    m_gbExceptions = new KExceptionBox( page, "m_gbExceptions" );
    m_gbExceptions->setSizePolicy( QSizePolicy(QSizePolicy::Expanding,
                                              QSizePolicy::Preferred,
                                              m_gbExceptions->sizePolicy().hasHeightForWidth()) );
    mainLayout->addWidget( m_gbExceptions );

    QHBoxLayout* hlay = new QHBoxLayout;
    hlay->setSpacing( KDialog::spacingHint() );
    hlay->setMargin( 0 );
    spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum,
                              QSizePolicy::Minimum );
    hlay->addItem( spacer );

    init();
}

KManualProxyDlg::~KManualProxyDlg()
{
}

void KManualProxyDlg::init()
{
    m_sbHttp->setRange( 0, MAX_PORT_VALUE );
    m_sbHttps->setRange( 0, MAX_PORT_VALUE );
    m_sbFtp->setRange( 0, MAX_PORT_VALUE );

    // Enable entries
    connect( m_cbHttp, SIGNAL( toggled(bool) ), m_leHttp,
             SLOT( setEnabled(bool) ) );
    connect( m_cbHttps, SIGNAL( toggled(bool) ), m_leHttps,
             SLOT( setEnabled(bool) ) );
    connect( m_cbFtp, SIGNAL( toggled(bool) ), m_leFtp,
             SLOT( setEnabled(bool) ) );

    // Enable port settings
    connect( m_cbHttp, SIGNAL( toggled(bool) ), m_sbHttp,
             SLOT( setEnabled(bool) ) );
    connect( m_cbHttps, SIGNAL( toggled(bool) ), m_sbHttps,
             SLOT( setEnabled(bool) ) );
    connect( m_cbFtp, SIGNAL( toggled(bool) ), m_sbFtp,
             SLOT( setEnabled(bool) ) );

    connect( m_cbHttp, SIGNAL( toggled(bool) ), SLOT( setChecked(bool) ) );
    connect( m_cbHttps, SIGNAL( toggled(bool) ), SLOT( setChecked(bool) ) );
    connect( m_cbFtp, SIGNAL( toggled(bool) ), SLOT( setChecked(bool) ) );

    connect( m_pbCopyDown, SIGNAL( clicked() ), SLOT( copyDown() ) );

    setChecked( true );
}

void KManualProxyDlg::setProxyData( const KProxyData &data )
{
    if ( data.type == KProtocolManager::NoProxy ||
         data.type == KProtocolManager::ManualProxy )
    {
        KURL u;
        int port;

        // Set the HTTP proxy
        u = data.httpProxy;
        m_cbHttp->setChecked( !data.httpProxy.isEmpty() &&
                                  u.isValid() );
        port = u.port();
        if ( port < 1 )
            port = DEFAULT_PROXY_PORT;

        u.setPort( 0 );

        if(m_cbHttp->isChecked())
            m_leHttp->setText( u.url() );

        m_sbHttp->setValue( port );

        // Set the HTTPS proxy
        u = data.httpsProxy;
        m_cbHttps->setChecked( !data.httpsProxy.isEmpty() &&
                                 u.isValid() );
        port = u.port();
        if ( port < 1 )
            port = DEFAULT_PROXY_PORT;

        u.setPort( 0 );

        if(m_cbHttps->isChecked())
            m_leHttps->setText( u.url() );

        m_sbHttps->setValue( port );

        // Set the FTP proxy...
        u = data.ftpProxy;
        m_cbFtp->setChecked( !data.ftpProxy.isEmpty() &&
                                 u.isValid() );
        port = u.port();
        if ( port < 1 )
            port = DEFAULT_PROXY_PORT;

        u.setPort( 0 );

        if(m_cbFtp->isChecked())
            m_leFtp->setText( u.url() );

        m_sbFtp->setValue( port );

        m_gbExceptions->fillExceptions( data.noProxyFor );
        m_gbExceptions->setCheckReverseProxy( data.useReverseProxy );
    }
    else
    {
       m_sbHttp->setValue( DEFAULT_PROXY_PORT );
       m_sbHttps->setValue( DEFAULT_PROXY_PORT );
       m_sbFtp->setValue( DEFAULT_PROXY_PORT );
    }
}

const KProxyData KManualProxyDlg::data() const
{
    KURL u;
    KProxyData data;

    if (!m_bHasValidData)
      return data;

    if ( m_cbHttp->isChecked() )
    {
        u = m_leHttp->text();
        if ( u.isValid() )
        {
            u.setPort( m_sbHttp->value() );
            data.httpProxy = u.url();
        }
    }
    if ( m_cbHttps->isChecked() )
    {
        u = m_leHttps->text();
        if ( u.isValid() )
        {
            u.setPort( m_sbHttps->value() );
            data.httpsProxy = u.url();
        }
    }
    if ( m_cbFtp->isChecked() )
    {
        u = m_leFtp->text();
        if ( u.isValid() )
        {
            u.setPort( m_sbFtp->value() );
            data.ftpProxy = u.url();
        }
    }

    QStringList list = m_gbExceptions->exceptions();
    if ( !list.isEmpty() )
        data.noProxyFor = list;

    data.type = KProtocolManager::ManualProxy;
    data.useReverseProxy = m_gbExceptions->isReverseProxyChecked();

    return data;
}

void KManualProxyDlg::setChecked( bool )
{
    bool checked = (m_cbHttp->isChecked() || m_cbHttps->isChecked() ||
                    m_cbFtp->isChecked());

    m_pbCopyDown->setEnabled( checked );
}


bool KManualProxyDlg::validate()
{
    KURL u;
    QFont f;

    if ( m_cbHttp->isChecked() )
    {
        u = m_leHttp->text();
        if ( !u.isValid() )
        {
            f = m_cbHttp->font();
            f.setBold( true );
            m_cbHttp->setFont( f );
        }
        else
          m_bHasValidData |= true;
    }
    if ( m_cbHttps->isChecked() )
    {
        u = m_leHttps->text();
        if ( !u.isValid() )
        {
            f = m_cbHttps->font();
            f.setBold( true );
            m_cbHttps->setFont( f );
        }
        else
          m_bHasValidData |= true;
    }
    if ( m_cbFtp->isChecked() )
    {
        u = m_leFtp->text();
        if ( !u.isValid() )
        {
            f = m_cbFtp->font();
            f.setBold( true );
            m_cbFtp->setFont( f );
        }
        else
            m_bHasValidData |= true;
    }

    if ( !m_bHasValidData )
    {
        QString msg = i18n("The highlighted input field(s) contain an invalid "
                           "proxy address.");

        QString details = i18n("<qt>Make sure the proxy address you "
                               "provided is valid. Note that you <b>must</b> "
                               "supply a fully qualified address such as "
                               "<b>http://192.168.20.1</b>. All addresses "
                               "specified without their protocols (eg: "
                               "\"http\") will be rejected as invalid.</qt>");

        KMessageBox::detailedError( this, msg, details,
                                    i18n("Invalid Proxy Setup") );
    }

    return m_bHasValidData;
}

void KManualProxyDlg::copyDown()
{
    int action = -1;
    bool isHttpChecked = m_cbHttp->isChecked();
    bool isHttpsChecked = m_cbHttps->isChecked();
    bool isFtpChecked = m_cbFtp->isChecked();

    if ( isHttpChecked )
        action += 4;
    else if ( isHttpsChecked )
        action += 3;
    else if ( isFtpChecked )
        action += 2;

    switch ( action )
    {
    case 3:
        if ( isHttpsChecked )
        {
            m_leHttps->setText( m_leHttp->text() );
            m_sbHttps->setValue( m_sbHttp->value() );
        }
        if ( isFtpChecked )
        {
            m_leFtp->setText( m_leHttp->text() );
            m_sbFtp->setValue( m_sbHttp->value() );
        }

        break;
    case 2:
        if ( isFtpChecked )
        {
            m_leFtp->setText( m_leHttps->text() );
            m_sbFtp->setValue( m_sbHttps->value() );
        }
        break;
    case 1:
    case 0:
    default:
        break;
    }
}

void KManualProxyDlg::slotOk()
{
    kdDebug() << "KManualProxyDlg::slotOk pressed" << endl;

    if ( validate() )
      KDialogBase::slotOk();
}

#include "kmanualproxydlg.moc"
