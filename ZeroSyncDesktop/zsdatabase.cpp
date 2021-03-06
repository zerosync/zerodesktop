/* =========================================================================
   ZSDatabase - Database class of ZeroSync desktop client


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


#include "zsdatabase.h"

ZSDatabase* ZSDatabase::m_Instance = 0;

ZSDatabase::ZSDatabase()
{
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(getDataBasePath());    
    if(!database.open())
    {
        qDebug() << "Error - ZSDatabase::ZSDatabase(QObject *parent) failed: " << database.lastError().text();
    }
    if(!tablesCreated())
    {
        createTables();
    }
}


QString ZSDatabase::getDataBasePath()
{
    QDir dir(QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0));
    if(!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.absolutePath().append("/zsdatabase.sqlite");
}


void ZSDatabase::createTables()
{
    QFile fileCreateTableFiles(":/sql/resources/sql/create_files.sql");
    QFile fileCreateTableIndex(":/sql/resources/sql/create_index.sql");

    if(!fileCreateTableFiles.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error - ZSDatabase::createTables() failed: Can't open resource file :sql/create_files.sql";
    }

    if(!fileCreateTableIndex.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error - ZSDatabase::createTables() failed: Can't open resource file :sql/create_index.sql";
    }

    if(database.open())
    {
        QString databaseQuery;
        QSqlQuery query(database);
        QTextStream inputStream(&fileCreateTableFiles);
        databaseQuery = inputStream.readAll();
        fileCreateTableFiles.close();
        if(!query.exec(databaseQuery))
        {
            qDebug() << "Error - ZSDatabase::createTables() failed to execute query from sql/create_files.sql: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::createTables() failed: " << database.lastError().text();
    }

    if(database.open())
    {
        QString databaseQuery;
        QSqlQuery query(database);
        QTextStream inputStream(&fileCreateTableIndex);
        databaseQuery = inputStream.readAll();
        fileCreateTableIndex.close();
        if(!query.exec(databaseQuery))
        {
            qDebug() << "Error - ZSDatabase::createTables() failed to execute query from sql/create_index.sql: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::createTables() failed: " << database.lastError().text();
    }
}


bool ZSDatabase::tablesCreated()
{
    QFile databaseFile(getDataBasePath());
    if(databaseFile.size() == 0)
    {
        databaseFile.remove();
        return false;
    }
    return true;
}

void ZSDatabase::deleteAllRowsFromFilesTable()
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("DELETE FROM files");
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::deleteAllRowsFromFilesTable() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::deleteAllRowsFromFilesTable() failed: " << database.lastError().text();
    }
    mutex.unlock();
}

void ZSDatabase::setZeroSyncFolderChangedFlagToFileIndexTable()
{
    mutex.lock();
    int state = getLatestState() + 1;
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("INSERT INTO fileindex (state, path, operation, timestamp, size, newpath, checksum) VALUES (:state, :path, :operation, :timestamp, :size, :newpath, :checksum)");
        query.bindValue(":state", state);
        query.bindValue(":path", "SET");
        query.bindValue(":operation", "SET");
        query.bindValue(":timestamp", 0);
        query.bindValue(":size", 0);
        query.bindValue(":newpath", "SET");
        query.bindValue(":checksum", "SET");
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setZeroSyncFolderChangedFlagToFileIndexTable() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setZeroSyncFolderChangedFlagToFileIndexTable() failed: " << database.lastError().text();
    }
    mutex.unlock();
}


void ZSDatabase::insertNewFile(QString path, qint64 timestamp, QString checksum, qint64 size)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("INSERT INTO files (path, timestamp, checksum, size, newpath, changed, updated, renamed, deleted, changed_self, reference) VALUES (:path, :timestamp, :checksum, :size, :newpath, :changed, :updated, :renamed, :deleted, :changed_self, :reference)");
        query.bindValue(":path", path);
        query.bindValue(":timestamp", timestamp);
        query.bindValue(":checksum", checksum);
        query.bindValue(":size", size);
        query.bindValue(":newpath", QString());
        query.bindValue(":changed", 1);
        query.bindValue(":updated", 1);
        query.bindValue(":renamed", 0);
        query.bindValue(":deleted", 0);
        query.bindValue(":changed_self", 0);
        query.bindValue(":reference", 0);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::insertNewFile() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::insertNewFile() failed: " << database.lastError().text();
    }
    mutex.unlock();
}


void ZSDatabase::setFileChanged(QString path, int value)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET changed = :value WHERE path = :path");
        query.bindValue(":value", value);
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setFileChanged() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setFileChanged() failed: " << database.lastError().text();
    }
    mutex.unlock();
}


void ZSDatabase::setFileUpdated(QString path, int value)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET updated = :value WHERE path = :path");
        query.bindValue(":value", value);
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setFileUpdated() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setFileUpdated() failed: " << database.lastError().text();
    }
    mutex.unlock();
}


void ZSDatabase::setFileRenamed(QString path, int value)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET renamed = :value WHERE path = :path");
        query.bindValue(":value", value);
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setFileRenamed() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setFileRenamed() failed: " << database.lastError().text();
    }
    mutex.unlock();
}

void ZSDatabase::setFileReference(QString path, quint32 value)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET reference = :value WHERE path = :path");
        query.bindValue(":value", value);
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setFileRenamed() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setFileRenamed() failed: " << database.lastError().text();
    }
    mutex.unlock();
}

void ZSDatabase::setFileDeleted(QString path, int value)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET deleted = :value WHERE path = :path");
        query.bindValue(":value", value);
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setFileDeleted() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setFileDeleted() failed: " << database.lastError().text();
    }
    mutex.unlock();
}

void ZSDatabase::setFileTimestamp(QString path, qint64 value)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET timestamp = :value WHERE path = :path");
        query.bindValue(":value", value);
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setFileTimestamp() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setFileTimestamp() failed: " << database.lastError().text();
    }
    mutex.unlock();
}

void ZSDatabase::setFileHashToZero(QString path)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET checksum = 0 WHERE path = :path");
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setFileHashToZero() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setFileHashToZero() failed: " << database.lastError().text();
    }
    mutex.unlock();
}


void ZSDatabase::setFileChangedSelf(QString path, int value)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET changed_self = :value WHERE path = :path");
        query.bindValue(":path", path);
        query.bindValue(":value", value);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setFileChangedSelf() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setFileChangedSelf() failed: " << database.lastError().text();
    }
    mutex.unlock();
}

void ZSDatabase::setNewPath(QString path, QString newPath)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET newpath = :newPath WHERE path = :path");
        query.bindValue(":newPath", newPath);
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setNewPath() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setNewPath() failed: " << database.lastError().text();
    }
    mutex.unlock();
}

bool ZSDatabase::isFileChanged(QString path)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE path = :path AND changed = 1");
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::isFileChanged() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return false;
        }
        if(query.next())
        {
            mutex.unlock();
            return true;
        }
        mutex.unlock();
        return false;
    }
    else
    {
        qDebug() << "Error - ZSDatabase::isFileChanged() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return false;
}

bool ZSDatabase::isFileChangedSelf(QString path)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE path = :path AND changed_self = 1");
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::isFileChangedSelf() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return false;
        }
        if(query.next())
        {
            mutex.unlock();
            return true;
        }
        mutex.unlock();
        return false;
    }
    else
    {
        qDebug() << "Error - ZSDatabase::isFileChangedSelf() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return false;
}

bool ZSDatabase::isFileUpdated(QString path)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE path = :path AND updated = 1");
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::isFileUpdated() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return false;
        }
        if(query.next())
        {
            mutex.unlock();
            return true;
        }
        mutex.unlock();
        return false;
    }
    else
    {
        qDebug() << "Error - ZSDatabase::isFileUpdated() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return false;
}

bool ZSDatabase::isFileRenamed(QString path)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE path = :path AND renamed = 1");
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::isFileRenamed() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return false;
        }
        if(query.next())
        {
            mutex.unlock();
            return true;
        }
        mutex.unlock();
        return false;
    }
    else
    {
        qDebug() << "Error - ZSDatabase::isFileRenamed() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return false;
}

bool ZSDatabase::isFileDeleted(QString path)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE path = :path AND deleted = 1");
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::isFileDeleted() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return false;
        }
        if(query.next())
        {
            mutex.unlock();
            return true;
        }
        mutex.unlock();
        return false;
    }
    else
    {
        qDebug() << "Error - ZSDatabase::isFileDeleted() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return false;
}

QString ZSDatabase::getFilePathForHash(QString hash)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE checksum = :checksum");
        query.bindValue(":checksum", hash);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::getFilePathForHash() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return QString();
        }
        if(query.next())
        {
            mutex.unlock();
            return query.value(0).toString();
        }
        mutex.unlock();
        return QString();
    }
    else
    {
        qDebug() << "Error - ZSDatabase::getFilePathForHash() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return QString();
}


void ZSDatabase::setFileMetaData(QString path, qint64 timestamp, QString checksum, qint64 size)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET timestamp = :timestamp, checksum = :checksum,  size = :size WHERE path = :path");
        query.bindValue(":path", path);
        query.bindValue(":timestamp", timestamp);
        query.bindValue(":checksum", checksum);
        query.bindValue(":size", size);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::setFileMetaData() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::setFileMetaData() failed: " << database.lastError().text();
    }
    mutex.unlock();
}


bool ZSDatabase::existsFileEntry(QString path)
{
    mutex.unlock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE path = :path");
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::existsFileEntry() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return false;
        }
        if(query.next())
        {
            mutex.unlock();
            return true;
        }
        mutex.unlock();
        return false;
    }
    else
    {
        qDebug() << "Error - ZSDatabase::existsFileEntry() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return false;
}

bool ZSDatabase::existsFileHash(QString checksum)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE checksum = :checksum");
        query.bindValue(":checksum", checksum);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::existsFileHash() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return false;
        }
        if(query.next())
        {
            mutex.unlock();
            return true;
        }
        mutex.unlock();
        return false;
    }
    else
    {
        qDebug() << "Error - ZSDatabase::existsFileHash() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return false;
}


QSqlQuery ZSDatabase::fetchAllChangedEntriesInFilesTable()
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE changed = 1");
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::fetchAllChangedEntriesInFilesTable() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return QSqlQuery();
        }
        else
        {
            mutex.unlock();
            return query;
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::fetchAllChangedEntriesInFilesTable() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return QSqlQuery();
}

QSqlQuery ZSDatabase::fetchUpdate(int lastest_state)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM fileindex WHERE state = :state AND changed_self = 0");
        query.bindValue(":state", lastest_state);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::fetchUpdate() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return QSqlQuery();
        }
        else
        {
            mutex.unlock();
            return query;
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::fetchUpdate() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return QSqlQuery();
}

QSqlQuery ZSDatabase::fetchUpdateFromState(int fromState)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM fileindex WHERE state > :from");
        query.bindValue(":from", fromState);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::fetchUpdateFromState() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return QSqlQuery();
        }
        else
        {
            mutex.unlock();
            return query;
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::fetchUpdateFromState() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return QSqlQuery();
}

QSqlQuery ZSDatabase::fetchAllEntriesInFilesTable()
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files");
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::fetchAllEntriesInFilesTable() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return QSqlQuery();
        }
        else
        {
            mutex.unlock();
            return query;
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::fetchAllEntriesInFilesTable() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return QSqlQuery();
}

QSqlQuery ZSDatabase::fetchFileByPath(QString path)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE path = :path");
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::fetchFileByPath() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return QSqlQuery();
        }
        else
        {
            mutex.unlock();
            return query;
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::fetchFileByPath() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return QSqlQuery();

}


void ZSDatabase::insertNewIndexEntry(int state, QString path, QString operation, qint64 timestamp, qint64 size, QString newpath, QString checksum, int changed_self)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("INSERT INTO fileindex (state, path, operation, timestamp, size, newpath, checksum, changed_self) VALUES (:state, :path, :operation, :timestamp, :size, :newpath, :checksum, :changed_self)");
        query.bindValue(":state", state);
        query.bindValue(":path", path);
        query.bindValue(":operation", operation);
        query.bindValue(":timestamp", timestamp);
        query.bindValue(":size", size);
        query.bindValue(":newpath", newpath);
        query.bindValue(":checksum", checksum);
        query.bindValue(":changed_self", changed_self);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::insertNewIndexEntry() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::insertNewIndexEntry() failed: " << database.lastError().text();
    }
    mutex.unlock();
}

int ZSDatabase::getLatestState()
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT MAX(state) FROM fileindex");
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::getLatestState() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return -1;
        }
        if(query.next())
        {
            mutex.unlock();
            return query.value(0).toInt();
        }
        mutex.unlock();
        return 0;
    }
    else
    {
        qDebug() << "Error - ZSDatabase::getLatestState() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return -1;
}

qint64 ZSDatabase::getTimestampForFile(QString path)
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT timestamp FROM files WHERE path = :path");
        query.bindValue(":path", path);
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::getTimestampForFile() failed to execute query: " << query.lastError().text();
            mutex.unlock();
            return -1;
        }
        if(query.next())
        {
            mutex.unlock();
            return query.value(0).toLongLong();
        }
        mutex.unlock();
        return 0;
    }
    else
    {
        qDebug() << "Error - ZSDatabase::getTimestampForFile() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return -1;
}

QSqlQuery ZSDatabase::fetchAllUndeletedEntries()
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("SELECT * FROM files WHERE deleted = 0");
        if(!query.exec())
        {
            qDebug() << "Error: Can't execute database query to fetch all undeleted files";
            mutex.unlock();
            return QSqlQuery();
        }
        else
        {
            mutex.unlock();
            return query;
        }
    }else {
        qDebug() << "Error: fetAllundeletedEntries() failed: " << database.lastError().text();
    }
    mutex.unlock();
    return QSqlQuery();
}


void ZSDatabase::resetFileMetaData()
{
    mutex.lock();
    if(database.open())
    {
        QSqlQuery query(database);
        query.prepare("UPDATE files SET changed = 0, updated = 0, changed_self = 0 WHERE changed = 1");
        if(!query.exec())
        {
            qDebug() << "Error - ZSDatabase::resetFileMetaData() failed to execute query: " << query.lastError().text();
        }
    }
    else
    {
        qDebug() << "Error - ZSDatabase::resetFileMetaData() failed: " << database.lastError().text();
    }
    mutex.unlock();
}
