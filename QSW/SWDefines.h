#ifndef SWDEFINES_H
#define SWDEFINES_H

#include <QtCore/QString>
#include <QtCore/QMetaType>

extern QString QSW_VERSION;
extern QString CLIENT_VERSION;
extern QString QSW_BUILD;
extern QString CLIENT_BUILD;

extern quint8  Locale;
extern QString ProcFlagDesc[];

#define MAX_PROC 32 
#define MAX_SPELL_REAGENTS 8
#define MAX_SPELL_TOTEMS 2

#define arrayLength(a, b) sizeof(a) / sizeof(b)

enum AttrType
{
    TYPE_ATTR,
    TYPE_ATTR_EX,
    TYPE_ATTR_EX2,
    TYPE_ATTR_EX3,
    TYPE_ATTR_EX4,
    TYPE_TARGETS,
    TYPE_CREATURE,
    TYPE_FORMS,
    TYPE_FORMS_NOT,
    TYPE_ITEM_WEAPON,
    TYPE_ITEM_ARMOR,
    TYPE_ITEM_MISC,
    TYPE_ITEM_INVENTORY,
    TYPE_CR,
    MAX_ATTR_TYPE
};

enum SpellEffectIndex
{
    EFFECT_INDEX_0     = 0,
    EFFECT_INDEX_1     = 1,
    EFFECT_INDEX_2     = 2,
    MAX_EFFECT_INDEX
};

#endif // SWDEFINES_H
