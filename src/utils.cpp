/***************************************************************************
                          utils.cpp  -  description
                             -------------------
    begin                : Wed Sep 24 2003
    copyright            : (C) 2003 by Max Howell
    email                : max.howell@methylblue.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>

#include <klocale.h>
#include <kglobal.h>

#include "define.h"
#include "filetree.h"


  
QString fullPath( const File *file, const Directory* const root )
{
  QString path;

  for( const Directory *p = (Directory *)file; (p != root && p != 0); p = p->parent() )
    path.prepend( p->name() );

  return path;
}


QString makeHumanReadable( unsigned int size, UnitPrefix key )
{
  QString s;
  double prettySize = (double)size / (double)UNIT_DENOMINATOR[ key ];
  KLocale *loc = KGlobal::locale();

  if( prettySize >= 0.01 )
  {
    if( prettySize < 1 )
      s = loc->formatNumber( prettySize, 2 );
    else if( prettySize < 100 )
      s = loc->formatNumber( prettySize, 1 );
    else
      s = loc->formatNumber( prettySize, 0 );

    s += ' ';
    s += UNIT_PREFIX[key];
    s += 'B';
  }

  if( prettySize < 0.1 )
  {
    s += " (";
    s += loc->formatNumber( size / UNIT_DENOMINATOR[key - 1], 0 );
    s += ' ';
    s += UNIT_PREFIX[key - 1];
    s += "B)";
  }

  return s;
}
