/*
   Copyright (C) 2005 Ivor Hewitt <ivor@ivor.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <kfiledialog.h>
#include <dcopclient.h>

#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include "filteropts.h"
#include "filteropts.moc"

KCMFilter::KCMFilter(KConfig *config, QString group,
                     QWidget *parent, const char *name )
    : KCModule( parent, name ),
      mConfig( config ),
      mGroupname( group ),
      mSelCount(0)
{
    setButtons(Default|Apply);

    QVBoxLayout *topLayout = new QVBoxLayout(this, 0, KDialog::spacingHint());

    mEnableCheck = new QCheckBox(i18n("Enable filters"), this);
    topLayout->addWidget( mEnableCheck );

    mKillCheck = new QCheckBox(i18n("Hide filtered images"), this);
    topLayout->addWidget( mKillCheck );

    QGroupBox *topBox = new QGroupBox( 1, Horizontal, i18n("URL Expressions to Filter"), this );
    topLayout->addWidget( topBox );

    mListBox = new QListBox( topBox );
    mListBox->setSelectionMode(QListBox::Extended);
    QLabel *label = new QLabel( i18n("Expression (e.g. http://www.site.com/ad/*):"), topBox);
    topLayout->addWidget(label);
    mString = new QLineEdit( topBox );

    QHBox *buttonBox = new QHBox( topBox );
    buttonBox->setSpacing( KDialog::spacingHint() );

    mInsertButton = new QPushButton( i18n("Insert"), buttonBox );
    connect( mInsertButton, SIGNAL( clicked() ), SLOT( insertFilter() ) );
    mUpdateButton = new QPushButton( i18n("Update"), buttonBox );
    connect( mUpdateButton, SIGNAL( clicked() ), SLOT( updateFilter() ) );
    mRemoveButton = new QPushButton( i18n("Remove"), buttonBox );
    connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeFilter() ) );

    mImportButton = new QPushButton(i18n("Import.."),buttonBox);
    connect( mImportButton, SIGNAL( clicked() ), SLOT( importFilters() ) );
    mExportButton = new QPushButton(i18n("Export.."),buttonBox);
    connect( mExportButton, SIGNAL( clicked() ), SLOT( exportFilters() ) );

    connect( mEnableCheck, SIGNAL( clicked()), this, SLOT( slotEnableChecked()));
    connect( mKillCheck, SIGNAL( clicked()), this, SLOT( slotKillChecked()));
    connect( mListBox, SIGNAL( selectionChanged ()),this, SLOT( slotItemSelected()));

    load();
    updateButton();
}

KCMFilter::~KCMFilter()
{
    delete mConfig;
}

void KCMFilter::slotKillChecked()
{
    emit changed( true );
}

void KCMFilter::slotEnableChecked()
{
    updateButton();
    emit changed( true );
}

void KCMFilter::slotItemSelected()
{
    int currentId=-1;
    int i;
    for( i=0,mSelCount=0; i < mListBox->count() && mSelCount<2; ++i )
    {
        if (mListBox->isSelected(i))
        {
            currentId=i;
            mSelCount++;
        }
    }

    if ( currentId >= 0 )
    {
        mString->setText(mListBox->text(currentId));
    }
    updateButton();
}

void KCMFilter::updateButton()
{
    bool state = mEnableCheck->isChecked();

    mUpdateButton->setEnabled(state && (mSelCount == 1));
    mRemoveButton->setEnabled(state && (mSelCount > 0));
    mInsertButton->setEnabled(state);
    mImportButton->setEnabled(state);
    mExportButton->setEnabled(state);

    mListBox->setEnabled(state);
    mString->setEnabled(state);
    mKillCheck->setEnabled(state);
}

void KCMFilter::importFilters()
{
  QString inFile = KFileDialog::getOpenFileName();
  if (inFile.length() > 0)
  {
    QFile f(inFile);
    if ( f.open( IO_ReadOnly ) )
    {
      QTextStream ts( &f );
      QStringList paths;
      QString line;
      while (!ts.atEnd())
      {
        line = ts.readLine();
        if ((!line.contains("[Adblock]")) &&
            (!line.startsWith("!")))
            paths.append(line);
      }
      f.close();

      mListBox->insertStringList( paths );
      emit changed(true);
    }
  }
}

void KCMFilter::exportFilters()
{
  QString outFile = KFileDialog::getSaveFileName();
  if (outFile.length() > 0)
  {
    QFile f(outFile);
    if ( f.open( IO_WriteOnly ) )
    {
      QTextStream ts( &f );
      ts.setEncoding( QTextStream::UnicodeUTF8 );
      ts << "[AdBlock]" << endl;

      uint i;
      for( i = 0; i < mListBox->count(); ++i )
        ts << mListBox->text(i) << endl;

      f.close();
    }
  }
}

void KCMFilter::defaults()
{
    mListBox->clear();
    updateButton();
}

void KCMFilter::save()
{
    mConfig->deleteGroup(mGroupname);
    mConfig->setGroup(mGroupname);

    mConfig->writeEntry("Enabled",mEnableCheck->isChecked());
    mConfig->writeEntry("Shrink",mKillCheck->isChecked());

    uint i;
    for( i = 0; i < mListBox->count(); ++i )
    {
        QString key = "Filter-" + QString::number(i);
        mConfig->writeEntry(key, mListBox->text(i));
    }
    mConfig->writeEntry("Count",mListBox->count());

    mConfig->sync();
    DCOPClient::mainClient()->send("konqueror*","KonquerorIface","reparseConfiguration()",QByteArray());

}

void KCMFilter::load()
{
    QStringList paths;

    mConfig->setGroup( mGroupname );
    mEnableCheck->setChecked( mConfig->readBoolEntry("Enabled",false));
    mKillCheck->setChecked( mConfig->readBoolEntry("Shrink",false));

    QMap<QString,QString> entryMap = mConfig->entryMap( mGroupname );
    QMap<QString,QString>::ConstIterator it;
    int num = mConfig->readNumEntry("Count",0);
    for (int i=0; i<num; ++i)
    {
        QString key = "Filter-" + QString::number(i);
        it = entryMap.find(key);
        if (it != entryMap.end())
            paths.append(it.data());
    }

    mListBox->insertStringList( paths );
}

void KCMFilter::insertFilter()
{
    if ( !mString->text().isEmpty() )
    {
        mListBox->insertItem( mString->text() );
        int id=mListBox->count()-1;
        mListBox->clearSelection();
        mListBox->setSelected(id,true);
        mListBox->setCurrentItem(id);
        mListBox->ensureCurrentVisible();
        emit changed( true );
    }
    updateButton();
}

void KCMFilter::removeFilter()
{
    for( int i = mListBox->count(); i >= 0; --i )
    {
        if (mListBox->isSelected(i))
            mListBox->removeItem(i);
    }
    emit changed( true );
    updateButton();
}

void KCMFilter::updateFilter()
{
    if ( !mString->text().isEmpty() )
    {
        int index = mListBox->currentItem();
        if ( index >= 0 )
        {
            mListBox->changeItem( mString->text(), index );
            emit changed( true );
        }
    }
    updateButton();
}

QString KCMFilter::quickHelp() const
{
    return i18n("<h1>Konqueror Filter</h1>");
}
