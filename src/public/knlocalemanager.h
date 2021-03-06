/*
 * Copyright (C) Kreogist Dev Team <kreogistdevteam@126.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */
#ifndef KNLOCALEMANAGER_H
#define KNLOCALEMANAGER_H

#include <QPixmap>
#include <QStringList>
#include <QObject>

class QTranslator;
class KNLocaleManager : public QObject
{
    Q_OBJECT
public:
    static KNLocaleManager *instance();
    QString currentLanguageName() const;
    QString currentLanguageID() const;
    QPixmap currentLanguageIcon() const;
    int languageSize() const;
    QString languageNameAt(const int &i);
    QPixmap languageIconAt(const int &i);
    void loadLanguageFiles();

    QString languageDirPath() const;
    void setLanguageDirPath(const QString &languageDirPath);

signals:
    void requireRetranslate();

public slots:
    void setLanguage(const int &index);
    void setLanguageFromID(const QString &id);

private:
    static KNLocaleManager *m_instance;
    explicit KNLocaleManager(QObject *parent = 0);
    int m_currentIndex=-1;
    struct LanguageItem
    {
        QString name;
        QString id;
        QString filePath;
        QPixmap icon;
    };
    QString m_languageDirPath;
    QList<LanguageItem> m_languageList;
    QPixmap m_noImageIcon;
    QTranslator *m_translator;
};

#endif // KNLOCALEMANAGER_H
