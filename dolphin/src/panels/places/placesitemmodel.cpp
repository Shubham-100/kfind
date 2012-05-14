/***************************************************************************
 *   Copyright (C) 2012 by Peter Penz <peter.penz19@gmail.com>             *
 *                                                                         *
 *   Based on KFilePlacesModel from kdelibs:                               *
 *   Copyright (C) 2007 Kevin Ottens <ervin@kde.org>                       *
 *   Copyright (C) 2007 David Faure <faure@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "placesitemmodel.h"

#include <KBookmark>
#include <KBookmarkGroup>
#include <KBookmarkManager>
#include <KComponentData>
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KStandardDirs>
#include <KUser>
#include "placesitem.h"
#include <QAction>
#include <QDate>
#include <QTimer>

#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/OpticalDisc>
#include <Solid/OpticalDrive>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>

#ifdef HAVE_NEPOMUK
    #include <Nepomuk/ResourceManager>
#endif

PlacesItemModel::PlacesItemModel(QObject* parent) :
    KStandardItemModel(parent),
    m_nepomukRunning(false),
    m_hiddenItemsShown(false),
    m_availableDevices(),
    m_predicate(),
    m_bookmarkManager(0),
    m_systemBookmarks(),
    m_systemBookmarksIndexes(),
    m_hiddenItems(),
    m_hiddenItemToRemove(-1),
    m_saveBookmarksTimer(0)
{
#ifdef HAVE_NEPOMUK
    m_nepomukRunning = (Nepomuk::ResourceManager::instance()->initialized());
#endif
    const QString file = KStandardDirs::locateLocal("data", "kfileplaces/bookmarks.xml");
    m_bookmarkManager = KBookmarkManager::managerForFile(file, "kfilePlaces");

    createSystemBookmarks();
    initializeAvailableDevices();
    loadBookmarks();

    m_saveBookmarksTimer = new QTimer(this);
    m_saveBookmarksTimer->setInterval(100);
    m_saveBookmarksTimer->setSingleShot(true);
    connect(m_saveBookmarksTimer, SIGNAL(timeout()), this, SLOT(saveBookmarks()));
}

PlacesItemModel::~PlacesItemModel()
{
    saveBookmarks();
    qDeleteAll(m_hiddenItems);
    m_hiddenItems.clear();
}

PlacesItem* PlacesItemModel::createPlacesItem(const QString& text,
                                              const KUrl& url,
                                              const QString& iconName)
{
    const KBookmark bookmark = PlacesItem::createBookmark(m_bookmarkManager, text, url, iconName);
    return new PlacesItem(bookmark);
}

PlacesItem* PlacesItemModel::placesItem(int index) const
{
    return dynamic_cast<PlacesItem*>(item(index));
}

int PlacesItemModel::hiddenCount() const
{
    int modelIndex = 0;
    int itemCount = 0;
    foreach (const PlacesItem* hiddenItem, m_hiddenItems) {
        if (hiddenItem) {
            ++itemCount;
        } else {
            if (placesItem(modelIndex)->isHidden()) {
                ++itemCount;
            }
            ++modelIndex;
        }
    }

    return itemCount;
}

void PlacesItemModel::setHiddenItemsShown(bool show)
{
    if (m_hiddenItemsShown == show) {
        return;
    }

    m_hiddenItemsShown = show;

    if (show) {
        // Move all items that are part of m_hiddenItems to the model.
        int modelIndex = 0;
        for (int hiddenIndex = 0; hiddenIndex < m_hiddenItems.count(); ++hiddenIndex) {
            if (m_hiddenItems[hiddenIndex]) {
                PlacesItem* visibleItem = new PlacesItem(*m_hiddenItems[hiddenIndex]);
                delete m_hiddenItems[hiddenIndex];
                m_hiddenItems.removeAt(hiddenIndex);
                insertItem(modelIndex, visibleItem);
                Q_ASSERT(!m_hiddenItems[hiddenIndex]);
            }
            ++modelIndex;
        }
    } else {
        // Move all items of the model, where the "isHidden" property is true, to
        // m_hiddenItems.
        Q_ASSERT(m_hiddenItems.count() == count());
        for (int i = count() - 1; i >= 0; --i) {
            const PlacesItem* visibleItem = placesItem(i);
            if (visibleItem->isHidden()) {
                removeHiddenItem(i);
            }
        }
    }
#ifdef PLACESITEMMODEL_DEBUG
        kDebug() << "Changed visibility of hidden items";
        showModelState();
#endif
}

bool PlacesItemModel::hiddenItemsShown() const
{
    return m_hiddenItemsShown;
}

int PlacesItemModel::closestItem(const KUrl& url) const
{
    int foundIndex = -1;
    int maxLength = 0;

    for (int i = 0; i < count(); ++i) {
        const KUrl itemUrl = placesItem(i)->url();
        if (itemUrl.isParentOf(url)) {
            const int length = itemUrl.prettyUrl().length();
            if (length > maxLength) {
                foundIndex = i;
                maxLength = length;
            }
        }
    }

    return foundIndex;
}

QAction* PlacesItemModel::ejectAction(int index) const
{
    const PlacesItem* item = placesItem(index);
    if (item && item->device().is<Solid::OpticalDisc>()) {
        return new QAction(KIcon("media-eject"), i18nc("@item", "Eject '%1'", item->text()), 0);
    }

    return 0;
}

QAction* PlacesItemModel::teardownAction(int index) const
{
    const PlacesItem* item = placesItem(index);
    if (!item) {
        return 0;
    }

    Solid::Device device = item->device();
    const bool providesTearDown = device.is<Solid::StorageAccess>() &&
                                  device.as<Solid::StorageAccess>()->isAccessible();
    if (!providesTearDown) {
        return 0;
    }

    Solid::StorageDrive* drive = device.as<Solid::StorageDrive>();
    if (!drive) {
        drive = device.parent().as<Solid::StorageDrive>();
    }

    bool hotPluggable = false;
    bool removable = false;
    if (drive) {
        hotPluggable = drive->isHotpluggable();
        removable = drive->isRemovable();
    }

    QString iconName;
    QString text;
    const QString label = item->text();
    if (device.is<Solid::OpticalDisc>()) {
        text = i18nc("@item", "Release '%1'", label);
    } else if (removable || hotPluggable) {
        text = i18nc("@item", "Safely Remove '%1'", label);
        iconName = "media-eject";
    } else {
        text = i18nc("@item", "Unmount '%1'", label);
        iconName = "media-eject";
    }

    if (iconName.isEmpty()) {
        return new QAction(text, 0);
    }

    return new QAction(KIcon(iconName), text, 0);
}

void PlacesItemModel::requestEject(int index)
{
    const PlacesItem* item = placesItem(index);
    if (item) {
        Solid::OpticalDrive* drive = item->device().parent().as<Solid::OpticalDrive>();
        if (drive) {
            connect(drive, SIGNAL(ejectDone(Solid::ErrorType,QVariant,QString)),
                    this, SLOT(slotStorageTeardownDone(Solid::ErrorType,QVariant)));
            drive->eject();
        } else {

        }
    }
}

void PlacesItemModel::requestTeardown(int index)
{
    const PlacesItem* item = placesItem(index);
    if (item) {
        Solid::StorageAccess* access = item->device().as<Solid::StorageAccess>();
        if (access) {
            connect(access, SIGNAL(teardownDone(Solid::ErrorType,QVariant,QString)),
                    this, SLOT(slotStorageTeardownDone(Solid::ErrorType,QVariant)));
            access->teardown();
        } else {
            const QString label = item->text();
            const QString message = i18nc("@info", "The device '%1' is not a disk and cannot be ejected.", label);
            emit errorMessage(message);
        }
    }
}

void PlacesItemModel::onItemInserted(int index)
{
    const PlacesItem* insertedItem = placesItem(index);
    if (insertedItem) {
        // Take care to apply the PlacesItemModel-order of the inserted item
        // also to the bookmark-manager.
        const KBookmark insertedBookmark = insertedItem->bookmark();

        const PlacesItem* previousItem = placesItem(index - 1);
        KBookmark previousBookmark;
        if (previousItem) {
            previousBookmark = previousItem->bookmark();
        }

        m_bookmarkManager->root().moveBookmark(insertedBookmark, previousBookmark);
    }

    if (index == count() - 1) {
        // The item has been appended as last item to the list. In this
        // case assure that it is also appended after the hidden items and
        // not before (like done otherwise).
        m_hiddenItems.append(0);
        return;
    }

    int modelIndex = -1;
    int hiddenIndex = 0;
    while (hiddenIndex < m_hiddenItems.count()) {
        if (!m_hiddenItems[hiddenIndex]) {
            ++modelIndex;
            if (modelIndex + 1 == index) {
                break;
            }
        }
        ++hiddenIndex;
    }
    m_hiddenItems.insert(hiddenIndex, 0);

    m_saveBookmarksTimer->start();

#ifdef PLACESITEMMODEL_DEBUG
    kDebug() << "Inserted item" << index;
    showModelState();
#endif
}

void PlacesItemModel::onItemRemoved(int index, KStandardItem* removedItem)
{
    PlacesItem* placesItem = dynamic_cast<PlacesItem*>(removedItem);
    if (placesItem) {
        const KBookmark bookmark = placesItem->bookmark();
        m_bookmarkManager->root().deleteBookmark(bookmark);
    }

    const int removeIndex = hiddenIndex(index);
    Q_ASSERT(!m_hiddenItems[removeIndex]);
    m_hiddenItems.removeAt(removeIndex);

    m_saveBookmarksTimer->start();

#ifdef PLACESITEMMODEL_DEBUG
    kDebug() << "Removed item" << index;
    showModelState();
#endif
}

void PlacesItemModel::onItemChanged(int index, const QSet<QByteArray>& changedRoles)
{
    const PlacesItem* changedItem = placesItem(index);
    if (changedItem) {
        // Take care to apply the PlacesItemModel-order of the inserted item
        // also to the bookmark-manager.
        const KBookmark insertedBookmark = changedItem->bookmark();

        const PlacesItem* previousItem = placesItem(index - 1);
        KBookmark previousBookmark;
        if (previousItem) {
            previousBookmark = previousItem->bookmark();
        }

        m_bookmarkManager->root().moveBookmark(insertedBookmark, previousBookmark);
    }

    if (changedRoles.contains("isHidden")) {
        const PlacesItem* shownItem = placesItem(index);
        Q_ASSERT(shownItem);
        if (!m_hiddenItemsShown && shownItem->isHidden()) {
            m_hiddenItemToRemove = index;
            QTimer::singleShot(0, this, SLOT(removeHiddenItem()));
        }
    }
    m_saveBookmarksTimer->start();
}

void PlacesItemModel::slotDeviceAdded(const QString& udi)
{
    const Solid::Device device(udi);
    if (m_predicate.matches(device)) {
        m_availableDevices << udi;
        const KBookmark bookmark = PlacesItem::createDeviceBookmark(m_bookmarkManager, udi);
        appendItem(new PlacesItem(bookmark));
    }
}

void PlacesItemModel::slotDeviceRemoved(const QString& udi)
{
    if (!m_availableDevices.contains(udi)) {
        return;
    }

    for (int i = 0; i < m_hiddenItems.count(); ++i) {
        PlacesItem* item = m_hiddenItems[i];
        if (item && item->udi() == udi) {
            m_hiddenItems.removeAt(i);
            delete item;
            return;
        }
     }

     for (int i = 0; i < count(); ++i) {
         if (placesItem(i)->udi() == udi) {
             removeItem(i);
             return;
         }
     }
}

void PlacesItemModel::slotStorageTeardownDone(Solid::ErrorType error, const QVariant& errorData)
{
    if (error && errorData.isValid()) {
        emit errorMessage(errorData.toString());
    }
}

void PlacesItemModel::removeHiddenItem()
{
    removeHiddenItem(m_hiddenItemToRemove);
    m_hiddenItemToRemove = -1;
}


void PlacesItemModel::saveBookmarks()
{
    // TODO: Temporary deactivated until 100 % backward compatibility is provided
    // m_bookmarkManager->emitChanged(m_bookmarkManager->root());
}

void PlacesItemModel::loadBookmarks()
{
    KBookmarkGroup root = m_bookmarkManager->root();
    KBookmark bookmark = root.first();
    QSet<QString> devices = m_availableDevices;

    QSet<KUrl> missingSystemBookmarks;
    foreach (const SystemBookmarkData& data, m_systemBookmarks) {
        missingSystemBookmarks.insert(data.url);
    }

    // The bookmarks might have a mixed order of places, devices and search-groups due
    // to the compatibility with the KFilePlacesPanel. In Dolphin's places panel the
    // items should always be collected in one group so the items are collected first
    // in separate lists before inserting them.
    QList<PlacesItem*> placesItems;
    QList<PlacesItem*> recentlyAccessedItems;
    QList<PlacesItem*> searchForItems;
    QList<PlacesItem*> devicesItems;

    while (!bookmark.isNull()) {
        const QString udi = bookmark.metaDataItem("UDI");
        const KUrl url = bookmark.url();
        const QString appName = bookmark.metaDataItem("OnlyInApp");
        const bool deviceAvailable = devices.remove(udi);

        const bool allowedHere = (appName.isEmpty() || appName == KGlobal::mainComponent().componentName())
                                 && (m_nepomukRunning || (url.protocol() != QLatin1String("timeline") &&
                                                          url.protocol() != QLatin1String("search")));

        if ((udi.isEmpty() && allowedHere) || deviceAvailable) {
            PlacesItem* item = new PlacesItem(bookmark);
            if (deviceAvailable) {
                devicesItems.append(item);
            } else {
                if (missingSystemBookmarks.contains(url)) {
                    missingSystemBookmarks.remove(url);

                    // Apply the translated text to the system bookmarks, otherwise an outdated
                    // translation might be shown.
                    const int index = m_systemBookmarksIndexes.value(url);
                    item->setText(m_systemBookmarks[index].text);
                    item->setSystemItem(true);
                }

                switch (item->groupType()) {
                case PlacesItem::PlacesType:           placesItems.append(item); break;
                case PlacesItem::RecentlyAccessedType: recentlyAccessedItems.append(item); break;
                case PlacesItem::SearchForType:        searchForItems.append(item); break;
                case PlacesItem::DevicesType:
                default:                               Q_ASSERT(false); break;
                }
            }
        }

        bookmark = root.next(bookmark);
    }

    if (!missingSystemBookmarks.isEmpty()) {
        // The current bookmarks don't contain all system-bookmarks. Add the missing
        // bookmarks.
        foreach (const SystemBookmarkData& data, m_systemBookmarks) {
            if (missingSystemBookmarks.contains(data.url)) {
                KBookmark bookmark = PlacesItem::createBookmark(m_bookmarkManager,
                                                                data.text,
                                                                data.url,
                                                                data.icon);

                const QString protocol = data.url.protocol();
                if (protocol == QLatin1String("timeline") || protocol == QLatin1String("search")) {
                    // As long as the KFilePlacesView from kdelibs is available, the system-bookmarks
                    // for timeline and search should be a Dolphin-specific setting.
                    bookmark.setMetaDataItem("OnlyInApp", KGlobal::mainComponent().componentName());
                }

                PlacesItem* item = new PlacesItem(bookmark);
                item->setSystemItem(true);

                switch (item->groupType()) {
                case PlacesItem::PlacesType:           placesItems.append(item); break;
                case PlacesItem::RecentlyAccessedType: recentlyAccessedItems.append(item); break;
                case PlacesItem::SearchForType:        searchForItems.append(item); break;
                case PlacesItem::DevicesType:
                default:                               Q_ASSERT(false); break;
                }
            }
        }
    }

    addItems(placesItems);
    addItems(recentlyAccessedItems);
    addItems(searchForItems);

    // Create items for devices that have not stored as bookmark yet
    foreach (const QString& udi, devices) {
        const KBookmark bookmark = PlacesItem::createDeviceBookmark(m_bookmarkManager, udi);
        devicesItems.append(new PlacesItem(bookmark));
    }

    addItems(devicesItems);

#ifdef PLACESITEMMODEL_DEBUG
    kDebug() << "Loaded bookmarks";
    showModelState();
#endif
}

void PlacesItemModel::addItems(const QList<PlacesItem*>& items)
{
    foreach (PlacesItem* item, items) {
        if (!m_hiddenItemsShown && item->isHidden()) {
            m_hiddenItems.append(item);
        } else {
            appendItem(item);
        }
    }
}

void PlacesItemModel::createSystemBookmarks()
{
    Q_ASSERT(m_systemBookmarks.isEmpty());
    Q_ASSERT(m_systemBookmarksIndexes.isEmpty());

    const QString timeLineIcon = "package_utility_time"; // TODO: Ask the Oxygen team to create
                                                         // a custom icon for the timeline-protocol

    m_systemBookmarks.append(SystemBookmarkData(KUrl(KUser().homeDir()),
                                                "user-home",
                                                i18nc("@item", "Home")));
    m_systemBookmarks.append(SystemBookmarkData(KUrl("remote:/"),
                                                "network-workgroup",
                                                i18nc("@item", "Network")));
    m_systemBookmarks.append(SystemBookmarkData(KUrl("/"),
                                                "folder-red",
                                                i18nc("@item", "Root")));
    m_systemBookmarks.append(SystemBookmarkData(KUrl("trash:/"),
                                                "user-trash",
                                                i18nc("@item", "Trash")));

    if (m_nepomukRunning) {
        m_systemBookmarks.append(SystemBookmarkData(KUrl("timeline:/today"),
                                                    timeLineIcon,
                                                    i18nc("@item Recently Accessed", "Today")));
        m_systemBookmarks.append(SystemBookmarkData(KUrl("timeline:/yesterday"),
                                                    timeLineIcon,
                                                    i18nc("@item Recently Accessed", "Yesterday")));
        m_systemBookmarks.append(SystemBookmarkData(KUrl("timeline:/thismonth"),
                                                    timeLineIcon,
                                                    i18nc("@item Recently Accessed", "This Month")));
        m_systemBookmarks.append(SystemBookmarkData(KUrl("timeline:/lastmonth"),
                                                    timeLineIcon,
                                                    i18nc("@item Recently Accessed", "Last Month")));
        m_systemBookmarks.append(SystemBookmarkData(KUrl("search:/documents"),
                                                    "folder-txt",
                                                    i18nc("@item Commonly Accessed", "Documents")));
        m_systemBookmarks.append(SystemBookmarkData(KUrl("search:/images"),
                                                    "folder-image",
                                                    i18nc("@item Commonly Accessed", "Images")));
        m_systemBookmarks.append(SystemBookmarkData(KUrl("search:/audio"),
                                                    "folder-sound",
                                                    i18nc("@item Commonly Accessed", "Audio Files")));
        m_systemBookmarks.append(SystemBookmarkData(KUrl("search:/videos"),
                                                    "folder-video",
                                                    i18nc("@item Commonly Accessed", "Videos")));
    }

    for (int i = 0; i < m_systemBookmarks.count(); ++i) {
        m_systemBookmarksIndexes.insert(m_systemBookmarks[i].url, i);
    }
}

void PlacesItemModel::initializeAvailableDevices()
{
    m_predicate = Solid::Predicate::fromString(
        "[[[[ StorageVolume.ignored == false AND [ StorageVolume.usage == 'FileSystem' OR StorageVolume.usage == 'Encrypted' ]]"
        " OR "
        "[ IS StorageAccess AND StorageDrive.driveType == 'Floppy' ]]"
        " OR "
        "OpticalDisc.availableContent & 'Audio' ]"
        " OR "
        "StorageAccess.ignored == false ]");
    Q_ASSERT(m_predicate.isValid());

    Solid::DeviceNotifier* notifier = Solid::DeviceNotifier::instance();
    connect(notifier, SIGNAL(deviceAdded(QString)),   this, SLOT(slotDeviceAdded(QString)));
    connect(notifier, SIGNAL(deviceRemoved(QString)), this, SLOT(slotDeviceRemoved(QString)));

    const QList<Solid::Device>& deviceList = Solid::Device::listFromQuery(m_predicate);
    foreach(const Solid::Device& device, deviceList) {
        m_availableDevices << device.udi();
    }
}

int PlacesItemModel::hiddenIndex(int index) const
{
    int hiddenIndex = 0;
    int visibleItemIndex = 0;
    while (hiddenIndex < m_hiddenItems.count()) {
        if (!m_hiddenItems[hiddenIndex]) {
            if (visibleItemIndex == index) {
                break;
            }
            ++visibleItemIndex;
        }
        ++hiddenIndex;
    }

    return hiddenIndex >= m_hiddenItems.count() ? -1 : hiddenIndex;
}

void PlacesItemModel::removeHiddenItem(int index)
{
    const PlacesItem* shownItem = placesItem(index);
    const int newIndex = hiddenIndex(index);
    if (shownItem && newIndex >= 0) {
        PlacesItem* hiddenItem = new PlacesItem(*shownItem);
        const KBookmark hiddenBookmark = hiddenItem->bookmark();

        const PlacesItem* previousItem = placesItem(index - 1);
        KBookmark previousBookmark;
        if (previousItem) {
            previousBookmark = previousItem->bookmark();
        }

        removeItem(index);

        // removeItem() also removed the bookmark from m_bookmarkManager in
        // PlacesItemModel::onItemRemoved(). However for hidden items the
        // bookmark should still be remembered, so readd it again:
        m_bookmarkManager->root().addBookmark(hiddenBookmark);
        m_bookmarkManager->root().moveBookmark(hiddenBookmark, previousBookmark);

        m_hiddenItems.insert(newIndex, hiddenItem);
        m_saveBookmarksTimer->start();
    }
}

#ifdef PLACESITEMMODEL_DEBUG
void PlacesItemModel::showModelState()
{
    kDebug() << "hidden-index   model-index   text";
    int j = 0;
    for (int i = 0; i < m_hiddenItems.count(); ++i) {
        if (m_hiddenItems[i]) {
            kDebug() <<  i << "(Hidden)    " << "             " << m_hiddenItems[i]->dataValue("text").toString();
        } else {
            if (item(j)) {
                kDebug() <<  i << "            " << j << "           " << item(j)->dataValue("text").toString();
            } else {
                kDebug() <<  i << "            " << j << "           " << "(not available yet)";
            }
            ++j;
        }
    }
}
#endif

#include "placesitemmodel.moc"
