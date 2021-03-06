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
#ifndef KNMUSICGENREMODEL_H
#define KNMUSICGENREMODEL_H

#include <QHash>

#include "knmusiccategorymodel.h"

class KNMusicGenreModel : public KNMusicCategoryModel
{
    Q_OBJECT
public:
    explicit KNMusicGenreModel(QObject *parent = 0);
    QIcon genreIcon(const QString &genreName);

signals:

public slots:
    void onCoverImageUpdate(const QString &categoryText,
                            const QString &imageKey,
                            const QPixmap &image);
    void onCategoryRecover(const QList<QStandardItem *> &musicRow);
    void onImageRecoverComplete(KNHashPixmapList *pixmapList);

protected:
    QStandardItem *generateItem(const QString &itemText,
                                const QPixmap &itemIcon=QPixmap());

private:
    inline void loadGenreIcons();
    QHash<QString, QIcon> m_genreIconMap;
};

#endif // KNMUSICGENREMODEL_H
