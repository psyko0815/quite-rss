/*! \file db_func.h ***********************************************************/

#ifndef DB_FUNC_H
#define DB_FUNC_H

extern QString kDbName;
extern QString kDbVersion;

/*! Инициализация файла базы. Если базы нет то база создаётся по шаблону.
 * Если база старой версии, то она преобразовывается к актуальной версии
 * \param[in] dbFileName путь файла базы
 ******************************************************************************/
QString initDB(const QString dbFileName);

#endif
