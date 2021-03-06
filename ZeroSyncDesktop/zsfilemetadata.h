/* =========================================================================
   ZSFileMetaData - Class to represent file metadata for the ZeroSync client


   -------------------------------------------------------------------------
   Copyright (c) 2014 Tommy Bluhm
   Copyright other contributors as noted in the AUTHORS file.

   This file is part of ZeroSync, see http://zerosync.org.

   This is free software; you can redistribute it and/or modify it under
   the terms of the GNU Lesser General Public License as published by the
   Free Software Foundation; either version 3 of the License, or (at your
   option) any later version.
   This software is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTA-
   BILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
   Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program. If not, see http://www.gnu.org/licenses/.
   =========================================================================
*/


#ifndef ZSFILEMETADATA_H
#define ZSFILEMETADATA_H

#include <QObject>
#include <QtDebug>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QFile>
#include <QByteArray>
#include <QDateTime>


//!  Class that provides file informations
/*!
  This Class is used to provide informations about files that will be saved to the
  local database.
*/
class ZSFileMetaData : public QObject
{
    Q_OBJECT
public:
    //!  Constructor
    /*!
      The default constructor.
    */
    explicit ZSFileMetaData(QObject *parent = 0, QString path = QString(), QString pathToZeroSyncDirectory = QString());
    QString getFilePath();
    qint64 getLastModified();
    QString getHash();
    qint64 getFileSize();
    bool existsFile(QString);

private:
    QString filePath;
    qint64 fileLastModified;
    QString hashOfFile;
    qint64 fileSize;

    void updateFileMetaData(QString, QString);
    QString calculateHash(QString);

signals:

public slots:

};

#endif // ZSFILEMETADATA_H
