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
#ifndef KNMUSICCATEGORYMODEL_H
#define KNMUSICCATEGORYMODEL_H

#include <QStandardItemModel>

#include "knmusicglobal.h"

using namespace KNMusic;

class KNHashPixmapList;
class KNMusicCategoryModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit KNMusicCategoryModel(QObject *parent = 0);
    QString noCategoryText() const;
    void setNoCategoryText(const QString &noCategoryText);
    int categoryIndex() const;
    void setCategoryIndex(int categoryIndex);
    QIcon noAlbumIcon() const;
    void setNoAlbumIcon(const QPixmap &noAlbumIcon);
    bool updateAlbumArt() const;
    void setUpdateAlbumArt(bool updateAlbumArt);
    void changeAlbumArt(const QModelIndex &target,
                        const QString &artworkKey,
                        const QIcon &artwork);

signals:
    void categoryAlbumArtUpdate(QModelIndex updatedIndex);

public slots:
    virtual void onCategoryAdded(const QList<QStandardItem *> &musicRow);
    virtual void onCategoryRemoved(const QList<QStandardItem *> &musicRow);
    virtual void onCategoryRecover(const QList<QStandardItem *> &musicRow);
    virtual void onCoverImageUpdate(const QString &categoryText,
                                    const QString &imageKey,
                                    const QPixmap &image);
    virtual void onImageRecoverComplete(KNHashPixmapList *pixmapList);

protected:
    virtual QStandardItem *generateItem(const QString &itemText,
                                        const QPixmap &itemIcon=QPixmap());

private:
    inline void resetModel();
    inline void setAlbumArt(const QModelIndex &target,
                            const QString &artworkKey,
                            const QIcon &artwork)
    {
        //Update the artwork.
        setData(target, artwork, Qt::DecorationRole);
        //Update the artwork key.
        setData(target, artworkKey, CategoryArtworkKeyRole);
    }
    int m_categoryIndex=-1;
    bool m_updateAlbumArt=true;
    QIcon m_noAlbumIcon;
    QString m_noCategoryText;
};

#endif // KNMUSICCATEGORYMODEL_H
