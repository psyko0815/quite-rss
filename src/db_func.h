/* ============================================================
* QuiteRSS is a open-source cross-platform RSS/Atom news feeds reader
* Copyright (C) 2011-2013 QuiteRSS Team <quiterssteam@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
/*! \file db_func.h ***********************************************************/

#ifndef DB_FUNC_H
#define DB_FUNC_H

#include <QtCore>
#include <QtSql>

extern QString kDbName;
extern QString kDbVersion;

/*! Инициализация файла базы. Если базы нет то база создаётся по шаблону.
 * Если база старой версии, то она преобразовывается к актуальной версии
 * \param[in] dbFileName путь файла базы
 ******************************************************************************/
QString initDB(const QString dbFileName, QSettings *settings);

/** @brief Применение пользовательских фильтров
 * @param db - база данных
 * @param feedId - Id ленты
 * @param filterId - Id конкретного фильтра
 *---------------------------------------------------------------------------*/
void setUserFilter(int feedId, int filterId = -1);

#endif
