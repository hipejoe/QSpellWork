#include "datasource.h"

#include "mpq/MPQ.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>

#include <QDataStream>
DBCSource::DBCSource(QSharedPointer<FormatSource> format) :
    _format(format), _header(nullptr), _strings(nullptr), _minId(0), _maxId(0)
{
}

QVariantHash DBCSource::getEntry(quint32 id) const
{
    qint32 index = _indexes.indexOf(id);
    return (index == -1 ? QVariantHash() : getRecord(index));
}

QVariantHash DBCSource::getRecord(quint32 id) const
{
    return _data.value(id);
}

bool DBCSource::load()
{
    if (MPQ::mpqDir().isEmpty())
    {
        QFile file(DBC::dbcDir() + _format->name);
        if (file.open(QFile::ReadOnly))
        {
            _rawData = file.readAll();
            file.close();
        }
    }
    else
    {
        _rawData = MPQ::readFile(DBC::dbcDir() + _format->name);
    }

    if (_rawData.size() == 0)
    {
        qCritical() << "Cannot load DBC " << _format->name;
        return false;
    }

    _header = reinterpret_cast<const DBCFileHeader *>(_rawData.constData());

    if (qstrncmp(_header->magic, DBC_MAGIC, 4) != 0)
    {
        qCritical() << "File " << _format->name << " is not a valid DBC file!";
        return false;
    }

    _strings = _rawData.constData() + sizeof(DBCFileHeader) + _header->recordCount * _header->recordSize;

    QByteArray records = _rawData.mid(20, _header->recordCount * _header->recordSize);
    QDataStream stream(&records, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::ByteOrder(QSysInfo::ByteOrder));

    union
    {
        quint32 uvalue;
        qint32 ivalue;
        float fvalue;
    } value;

    for (quint32 i = 0; i < _header->recordCount; ++i)
    {
        QVariantHash record;
        record.reserve(_header->fieldCount);

        for (quint32 j = 0; j < _header->fieldCount; ++j)
        {
            FormatField field = _format->fields.at(j);
            if (!field.load)
            {
                stream.skipRawData(4);
                continue;
            }
            switch (field.type)
            {
                case FormatType::TypeUint:
                {
                    stream >> value.uvalue;
                    record[field.name] = value.uvalue;
                    break;
                }
                case FormatType::TypeInt:
                {
                    stream >> value.uvalue;
                    record[field.name] = value.ivalue;
                    break;
                }
                case FormatType::TypeFloat:
                {
                    stream >> value.uvalue;;
                    record[field.name] = value.fvalue;
                    break;
                }
                case FormatType::TypeString:
                {
                    stream >> value.uvalue;;
                    record[field.name] = value.uvalue;//QString::fromUtf8(_strings + value.uvalue);
                    break;
                }
                default:
                {
                    stream >> value.uvalue;;
                    record[field.name] = value.uvalue;;
                    break;
                }
            }

            if (j == 0)
            {
                _indexes << value.uvalue;
            }
        }
        _data[i] = record;
    }

    _minId = *std::min_element(_indexes.begin(), _indexes.end());
    _maxId = *std::max_element(_indexes.begin(), _indexes.end());

    _rawData.clear();
    return true;
}

SQLSource::SQLSource(QSharedPointer<FormatSource> format) :
    _format(format), _recordCount(0), _minId(0), _maxId(0), _strings({QString()})
{
}

SQLSource::~SQLSource()
{
    qDebug() << "Destroyed!";
}

QVariantHash SQLSource::getEntry(quint32 id) const
{
    //qint32 index = _indexes.indexOf(id);
    return QVariantHash();//(index == -1 ? QVariantHash() : getRecord(index));
}

QVariantHash SQLSource::getRecord(quint32 id) const
{
    return QVariantHash();//_data.value(id);
}

SourceRecord* SQLSource::getEntry2(quint32 id) const
{
    qint32 index = _indexes.indexOf(id);
    return (index == -1 ? nullptr : getRecord2(index));
}

SourceRecord* SQLSource::getRecord2(quint32 id) const
{
    return _data.value(id);
}

bool SQLSource::load()
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query = db.exec(QString("SELECT * FROM %0 ORDER BY 1 ASC").arg(_format->name));

    _recordCount = query.size();

    if (!_recordCount)
    {
        qCritical() << "Cannot load SQL table " << _format->name;
        return false;
    }

    while (query.next())
    {
        QVariantHash record;
        for (qint32 i = 0; i < _format->fields.size(); ++i)
        {
            FormatField field = _format->fields.at(i);
            if (!field.load)
            {
                continue;
            }

            if (field.type == FormatType::TypeString)
            {
                QString str = query.value(i).toString();
                qint32 strIdx = _strings.indexOf(str);
                if (strIdx == -1)
                {
                    strIdx = _strings.size();
                    _strings << str;
                }
                record[field.name] = strIdx;
            }
            else
            {
                record[field.name] = query.value(i);
            }
        }
        _data[query.at()] = new SourceRecord(this, record);
        _indexes << query.value(0).toUInt();
    }

    _minId = *std::min_element(_indexes.begin(), _indexes.end());
    _maxId = *std::max_element(_indexes.begin(), _indexes.end());

    return true;
}