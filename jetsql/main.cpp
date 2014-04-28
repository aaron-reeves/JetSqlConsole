/*
JetSqlConsole

main.cpp
--------
Begin: 2004/06/21
Project: JetSqlConsole
Website: http://www.aaronreeves.com/jetsqlconsole
Author: Aaron Reeves <development@reevesdigital.com>
----------------------------------------------------
Copyright (C) 2004 - 2014 Aaron Reeves

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include <qdebug.h>
#include <qstring.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlist.h>
#include <qchar.h>

#include <ar_general_purpose/csql.h>
#include <ar_general_purpose/cqstringlist.h>
#include <ar_general_purpose/ccmdline.h>
#include <ar_general_purpose/qcout.h>

#define VERSION_NUMBER "1.2.0"

#define NUM_INSERT_QUERIES 199

typedef QList<int> TIntArray;

enum FormatTypes {
  FmtTable,
  FmtList
};



struct SSqlCommand {
  QString cmdText;
  int formatType;
};

// Forward declarations...
void showVersion( void );
void showHelp( void );

QString getDbPath( const bool isDbf ) {
  if( isDbf )
    cout  << endl << "Enter the path of a DBase database: " << flush;
  else
    cout  << endl << "Enter the path of an Access database: " << flush;

  return cin.readLine();
}



bool isSqlComment( QString cmd ) {
  cmd = cmd.trimmed();
  
  if( 0 == cmd.length() || "--" == cmd.left(2) || "#" == cmd.left(1) ) {
    return true;
  }
  else {
    return false;
  }
}



SSqlCommand getSqlCmd( void ) {
  SSqlCommand sqlCmdStructure;
  QString sqlCmd = QString();
  QString strRead;

  cout  << endl << "jetsql> " << flush;

  while( true ) {
    strRead = cin.readLine();
    sqlCmd.append( " " );
    sqlCmd.append( strRead );
    sqlCmd = sqlCmd.trimmed();

    if( isSqlComment( sqlCmd ) ) {
      break;
    }
    // FIX ME: make checking for the semicolon a bit more robust
    // (i.e. allow it to occur in the middle of a line, as long as it isn't in quotes)
    else if( ";" == sqlCmd.right( 1 ) ) {
      sqlCmd = sqlCmd.left( sqlCmd.length() - 1 ).trimmed();
      sqlCmdStructure.formatType = FmtTable;
      break;
    }
    else if( "\\G" == sqlCmd.right( 2 ) ) {
      sqlCmd = sqlCmd.trimmed();
      sqlCmd = sqlCmd.left( sqlCmd.length() -  2 ).trimmed();
      sqlCmdStructure.formatType = FmtList;
      break;
    }
    else if(
      ( "quit" == sqlCmd.toLower().left( 4 ) )
      || ( "exit" == sqlCmd.toLower().left( 4 ) )
      || ( "\\q" == sqlCmd.toLower().left( 2 ) )
    ) {
      sqlCmd = "exit";
      break;
    }
    else if( "close outfile" == sqlCmd.toLower().left( 13 ) ) {
      sqlCmd = "close outfile";
      break;
    }
    else if( "close" == sqlCmd.toLower().left( 5 ) ) {
      sqlCmd = "close";
      break;
    }
    else if( "\\." == sqlCmd.left( 2 ) ) {
      sqlCmd = sqlCmd.right( sqlCmd.length() - 2 ).trimmed();
      sqlCmd.prepend( "FILE " );

      if( ";" == sqlCmd.right( 1 ) ) {
       sqlCmd = sqlCmd.left( sqlCmd.length() - 1 ).trimmed();
      }

      break;
    }
    else {
      cout << "     -> " << flush;
    }
  }

  sqlCmdStructure.cmdText = sqlCmd;
  return sqlCmdStructure;
}



// FIX ME: this function has no error checking.
// Replace it with a more robust approach.
QString extractTableName( QString sqlCmd ) {
  QStringList parts = sqlCmd.split( QRegExp( "\\s+" ) );
  return parts[ parts.count() - 1 ];
}



QString column( QString label, int len ) {
  QString col;
  int i;
  int lenDiff;

  if( label.length() <= len ) {
    // Prepend the leading space
    col = QString( " %1" ).arg( label ); // Note the leading space

    // Add spaces until desired length is reached
    lenDiff = len - label.length();
    for( i = 0; lenDiff > i; ++i ) {
      col.append( " " );
    }

    // Tack on the trailing space
    col.append( " " );
  }
  else {
    col = QString( " %1" ).arg( label ); // Note the leading space
    col = col.left( len - 2 );
    col = col + "... "; // Note the trailing space
  }

  return col;
}



QString header( TIntArray* arr ) {
  int i, j;
  int spaces;
  QString head = "";

  for( i = 0; arr->size() > i; ++i ) {
    head.append( "+" );
    spaces = arr->at( i );
    for( j = 0; (spaces+2) > j; ++j ) {
      head.append( "-" );
    }
  }

  head.append( "+" );

  return head;
}



bool showSystemTables( CSqlDatabase* db, QTextStream* stream ) {
  QString listItem;
  QString title;
  TIntArray* arr;
  int maxLen;
  int i;

  if( NULL == stream ) {
    //qDebug() << "No stream in showSystemTables()"; 
    return true;
  }

  QStringList* tableList = db->newSystemTableList();

  title = QString( "System_Tables_in_%1" ).arg( db->name() );

  // Loop through the list once to determine max length
  maxLen = title.length();

  for( i = 0; i < tableList->count(); ++i ) {
    listItem = tableList->at(i);
    if( listItem.length() > maxLen ) maxLen = listItem.length();
  }

  arr = new TIntArray();
  arr->append( maxLen );

  *stream << header( arr ) << endl;
  *stream << "|" << column( title, maxLen ) << "|" << endl;
  *stream << header( arr ) << endl;

  for( i = 0; i < tableList->count(); ++i ) {
    listItem = tableList->at(i);
    *stream << "|" <<  column( listItem, maxLen ) << "|" << endl;
  }

  *stream << header( arr ) << endl;
  *stream << tableList->count() << " rows in set" << endl;
  *stream << endl;
  *stream << flush;

  delete arr;
  delete tableList;

  return( true );
}



bool showTables( CSqlDatabase* db, QTextStream* stream ) {
  QString listItem;
  QString title;
  TIntArray* arr;
  int maxLen;
  int i;

  if( NULL == stream ) {
    //qDebug() << "No stream in showTables()";
    return true;
  }

  QStringList* tableList = db->newTableList();

  title = QString( "Tables_in_%1" ).arg( db->name() );

  // Loop through the list once to determine max length
  maxLen = title.length();

  for( i = 0; i < tableList->count(); ++i ) {
    listItem = tableList->at(i);
    if( listItem.length() > maxLen ) maxLen = listItem.length();
  }

  arr = new TIntArray();
  arr->append( maxLen );

  *stream << header( arr ) << endl;
  *stream << "|" << column( title, maxLen ) << "|" << endl;
  *stream << header( arr ) << endl;

  for( i = 0; i < tableList->count(); ++i ) {
    listItem = tableList->at(i);
    *stream << "|" <<  column( listItem, maxLen ) << "|" << endl;
  }

  *stream << header( arr ) << endl;
  *stream << tableList->count() << " rows in set" << endl;
  *stream << endl;
  *stream << flush;

  delete arr;
  delete tableList;

  return( true );
}


QString csvString( QString str ) {
  QString result;

  result = str.replace( '"', "\"\"" );
  result = result.prepend( '"' );
  result = result.append( '"' );

  return result;
}



bool isString( QString fieldType ) {
  QString s = fieldType.toLower();

  if(
    "text" == s.left(4)
    || "memo" == s.left(4)
    || "varchar" == s.left(7)
    || "char" == s.left(4)
    || "longtext" == s.left(8)
  )
    return true;
  else
    return false;
}



bool isBool( QString fieldType ) {
  return( "bit" == fieldType.toLower() );
}



bool isDateTime( QString fieldType ) {
  return( "datetime" == fieldType.toLower() );
}



QString processField( QString fieldVal, QString fieldType, const int dbFormat ) {
  if( isString( fieldType ) ) {
    return CSqlDatabase::sqlQuote( fieldVal, dbFormat );
  }
  else if( isDateTime( fieldType ) ) {
    return CSqlDatabase::sqlQuote( fieldVal, dbFormat );
  }
  else if( isBool( fieldType ) ) {
    if( ( 0 == fieldVal.compare( "0" ) ) || ( 0 == fieldVal.compare( "false", Qt::CaseInsensitive ) ) || 0 == fieldVal.left(1).compare( "f", Qt::CaseInsensitive ) )
      return CSqlDatabase::sqlBool( false, dbFormat );
    else
      return CSqlDatabase::sqlBool( true, dbFormat );
  }
  else {
    return fieldVal;
  }
}



bool dumpTableCsv( QString tableName, CSqlDatabase* db, QTextStream* stream ) {
  QStringList* list;
  CFieldInfoList* fieldInfoList;
  CSqlFieldInfo* fi;
  CSqlResult* res;
  bool result;
  QString query;
  CSqlRow* row;
  int i;
  int fc;

  if( NULL == stream ) {
    //qDebug() << "No stream in dumpTableCsv()";
    return true;
  }

  // Check that the table exists
  //----------------------------
  list = db->newTableList();

  if( !( list->contains( tableName ) ) ) {
    *stream << "ERROR: Table '" << tableName << "' doesn't exist" << endl << endl << flush;
    delete list;
    return( false );
  }

  // Get field information
  //----------------------
  fieldInfoList = db->newFieldInfoList( tableName, &result );

  // Dump the header
  //----------------
  fc = fieldInfoList->count();
  for( i = 0; i < fc; ++i ) {
    fi = fieldInfoList->at(i);
    *stream << '"' << fi->fieldName() << '"';

    if( i + 1 < fc )
      *stream << ",";
    else
      *stream << endl;
  }

  // Dump the data
  //--------------
  query = QString( "SELECT * FROM `%1`" ).arg( tableName );
  res = new CSqlResult( query, db );

  if( res->success() ) {
    fc = res->fieldCount();

    for( row = res->fetchArrayFirst(); NULL != row; row = res->fetchArrayNext() ) {
      for( i = 0; i < fc; ++i ) {
        if( row->field(i)->isNull() ) {
          // do nothing
        }
        else if( isString( fieldInfoList->at(i)->fieldTypeDescr() ) )
          *stream << csvString( *(row->field( i ) ) );
        else
          *stream << *(row->field( i ) );

        if( i + 1 < fc )
          *stream << ",";
        else
          *stream << "\n";
      }
    }
  }

  *stream << endl;
  *stream << flush;

  delete res;
  delete fieldInfoList;
  delete list;

  return( result );
}



// Write one insert query at a time.
void dumpDataAccess( QString tableName, CSqlDatabase* db, QTextStream* stream, CFieldInfoList* fieldInfoList ) {
  CSqlResult* res;
  QString query;
  CSqlRow* row;
  //CSqlRow* lastRow;
  QString insertPrefix;
  int i, fc;

  if( NULL == stream ) {
    qDebug() << "No stream in dumpDataAccess()";
    return;
  }

  query = QString( "SELECT * FROM `%1`" ).arg( tableName );
  res = new CSqlResult( query, db );

  if( res->success() ) {
    fc = res->fieldCount();

    insertPrefix = QString( "INSERT INTO `%1` ( " ).arg( tableName );
    for( i = 0; i < fc; ++i ) {
      insertPrefix.append( "`" );
      insertPrefix.append( res->fieldName( i ) );
      insertPrefix.append( "`" );

      if( i + 1 < fc )
        insertPrefix.append( ", " );
      else
        insertPrefix.append( " ) VALUES " );
    }

    //qDebug() << insertPrefix;
    //lastRow = res->fetchArrayLast();

    for( row = res->fetchArrayFirst(); NULL != row; row = res->fetchArrayNext() ) {
      //qDebug() << "Writing to stream...";

      *stream << insertPrefix;

      *stream << " ( ";
      for( i = 0; i < fc; ++i ) {
        if( row->field(i)->isNull() )
          *stream << "NULL";
        else
          *stream << processField( *(row->field( i )),  fieldInfoList->at(i)->fieldTypeDescr(), CSqlDatabase::DBMSAccess );

        if( i + 1 < fc )
          *stream << ", ";
        else {
          *stream << " );" << endl;
        }
      }
    }
  }

  delete res;
}


// Write up to NUM_INSERT_QUERIES "insert queries" at the same time.
void dumpDataMySqlOrPostgres( QString tableName, CSqlDatabase* db, QTextStream* stream, CFieldInfoList* fieldInfoList, const int dbFormat ) {
  CSqlResult* res;
  QString query;
  CSqlRow* row;
  CSqlRow* lastRow;
  QString insertPrefix;
  int i, fc, nRowsWritten;

  if( NULL == stream ) {
    //qDebug() << "No stream in dumpDataMySqlOrPostgres()";
    return;
  }

  // Remember that the SOURCE database is always MS Access.  Table name needs to be delimited properly.
  query = QString( "SELECT * FROM %1" ).arg( CSqlDatabase::delimitedName( tableName, CSqlDatabase::DBMSAccess ) );
  res = new CSqlResult( query, db );

  if( res->success() && ( 0 < res->numRows() ) ) {
    fc = res->fieldCount();

    // Change the way names are delimited for the DESTINATION database.
    insertPrefix = QString( "INSERT INTO %1 ( " ).arg( CSqlDatabase::delimitedName( tableName, dbFormat ) );

    for( i = 0; i < fc; ++i ) {
      insertPrefix.append( CSqlDatabase::delimitedName( res->fieldName( i ), dbFormat ) );

      if( i + 1 < fc )
        insertPrefix.append( ", " );
      else
        insertPrefix.append( ") VALUES\n" );
    }

    lastRow = res->fetchArrayLast();

    nRowsWritten = 0;
    for( row = res->fetchArrayFirst(); NULL != row; row = res->fetchArrayNext() ) {
      if( 0 == nRowsWritten )
        *stream << insertPrefix;

      *stream << " ( ";
      for( i = 0; i < fc; ++i ) {
        if( row->field(i)->isNull() )
          *stream << "NULL";
        else
          *stream << processField( *(row->field( i )), fieldInfoList->at(i)->fieldTypeDescr(), dbFormat );

        if( i + 1 < fc )
          *stream << ", ";
        else {
          if( lastRow != row ) {
            if( NUM_INSERT_QUERIES != nRowsWritten ) {
              *stream << " )," << endl;
              ++nRowsWritten;
            }
            else {
              *stream << ");" << endl << endl;
              nRowsWritten = 0;
            }
          }
          else
            *stream << " );" << endl;
        }
      }
    }
  }

  delete res;
}



QString convertTypeDescr( CSqlFieldInfo* fi, const int dbFormat ) {
  QString result;

  switch( dbFormat ) {
    case CSqlDatabase::DBMSAccess:
      result = fi->fieldTypeDescr();
      break;
    case CSqlDatabase::DBMySQL:
      if( isBool( fi->fieldTypeDescr() ) )
        result = "boolean";
      else if( "text" == fi->fieldTypeDescr().toLower().left( 4 ) )
        result = QString( "varchar(%1)" ).arg( fi->fieldSize() );
      else if( "memo" == fi->fieldTypeDescr().toLower().left( 4 ) )
        result = "mediumtext";
      else if( "longtext" == fi->fieldTypeDescr().toLower().left(8) )
        result = "mediumtext";
      else if( "long" == fi->fieldTypeDescr().toLower().left(4) )
        result = "int";
      else if( "counter" == fi->fieldTypeDescr().toLower().left(7) )
        result = "int auto_increment primary key";
      else if( "currency" == fi->fieldTypeDescr().toLower().left(8) )
        result = "double";
      else if( "datetime" == fi->fieldTypeDescr().toLower().left(8) ) {
        if( 0 == fi->fieldDescr().left(9).compare( "datefield", Qt::CaseInsensitive ) ) {
          result = "date";
        }
        else if( 0 == fi->fieldDescr().left(9).compare( "timefield", Qt::CaseInsensitive ) ) {
          result = "time";
        }
        else if( 0 == fi->fieldDescr().left(14).compare( "timestampfield", Qt::CaseInsensitive ) ) {
          result = "timestamp";
        }
        else {
          result = "datetime";
        }
      }
      else
        result = fi->fieldTypeDescr();
      break;
    case CSqlDatabase::DBPostgres:
      if( isBool( fi->fieldTypeDescr() ) )
        result = "boolean";
      else if( "text" == fi->fieldTypeDescr().toLower().left( 4 ) )
        result = QString( "varchar(%1)" ).arg( fi->fieldSize() );
      else if( "memo" == fi->fieldTypeDescr().toLower().left( 4 ) )
        result = "text";
      else if( "longtext" == fi->fieldTypeDescr().toLower().left(8) )
        result = "text";
      else if( "long" == fi->fieldTypeDescr().toLower().left(4) )
        result = "integer";
      else if( "counter" == fi->fieldTypeDescr().toLower().left(7) )
        result = "serial";
      else if( "currency" == fi->fieldTypeDescr().toLower().left(8) )
        result = "money";
      else if( "datetime" == fi->fieldTypeDescr().toLower().left(8) ) {
        if( 0 == fi->fieldDescr().left(9).compare( "datefield", Qt::CaseInsensitive ) ) {
          result = "date";
        }
        else if( 0 == fi->fieldDescr().left(9).compare( "timefield", Qt::CaseInsensitive ) ) {
          result = "time";
        }
        else {
          result = "timestamp";
        }
      }
      else
        result = fi->fieldTypeDescr();
      break;
    default:
      cout << "Type conversion not implemented for selected database type." << endl << flush;
      result = fi->fieldTypeDescr();
      break;
  }

  return result;
}


void dumpTableConstraintsSql( QString tableName, CSqlDatabase* db, QTextStream* stream, const int dbFormat ) {
  CSqlResult* primaryKeys = NULL;
  CSqlRow* pk;
  QString pkStr;
  QString pkName;

  CSqlResult* indices = NULL;
  int i;
  QStringList indicesHandled;
  CSqlRow* index;
  CSqlRow* iindex;
  QString indexStr;
  QString indexName;
  bool indexIsUnique;
  int nIndices;

  // Get primary keys and indices
  //------------------------------
  primaryKeys = new CSqlResult( db );
  primaryKeys->primaryKeys();
  indices = new CSqlResult( db );
  indices->indices();

  // Build and dump the primary key statement
  //-----------------------------------------
  pkStr = "";
  pk = primaryKeys->fetchArrayFirst();
  while( NULL != pk ) {
    if( 0 == pk->field("TABLE_NAME")->compare( tableName ) ) {
      pkName = *(pk->field("PK_NAME"));

      if( pkStr.isEmpty() )
        pkStr = pkStr.append( CSqlDatabase::delimitedName( *(pk->field("COLUMN_NAME")), dbFormat ) );
      else
        pkStr = pkStr.append( QString( ", %1" ).arg( CSqlDatabase::delimitedName( *(pk->field("COLUMN_NAME")), dbFormat ) ) );
    }

    pk = primaryKeys->fetchArrayNext();
  }

  if( 0 < pkStr.length() ) {
    *stream << "-- Primary key for table " << CSqlDatabase::delimitedName( tableName, dbFormat ) << ":" << endl;

    if( CSqlDatabase::DBPostgres == dbFormat ) {
      *stream << QString( "CREATE UNIQUE INDEX %1 ON %2 ( %3 );" ).arg( CSqlDatabase::delimitedName( pkName, dbFormat ) ).arg( CSqlDatabase::delimitedName( tableName, dbFormat ) ).arg( pkStr ) << endl;
      *stream << QString( "ALTER TABLE %1 ADD PRIMARY KEY USING INDEX %2;" ).arg( CSqlDatabase::delimitedName( tableName, dbFormat ) ).arg( CSqlDatabase::delimitedName( pkName, dbFormat ) ) << endl << endl;
    }
    else {
      *stream << "-- Primary keys are not yet generated for this database type." << endl;
    }
  }
  else {
    *stream << "-- There is no primary key for table " << CSqlDatabase::delimitedName( tableName, dbFormat ) << endl;
  }


  // Dump remaining indices
  //-----------------------
  // Because a table can have multiple indices and because indices can have multiple columns,
  // we need to loop over the list of indices twice, scanning by name of every index
  // to make sure that we have all of the columns included.

  nIndices = 0;

  for( i = 0; i < indices->numRows(); ++i ) {
    iindex = indices->fetchArray( i );

    // Are we on the right table?  If not, move on.
    if( 0!= iindex->field("TABLE_NAME")->compare( tableName ) ) {
      continue;
    }
    // We've already dealt with primary keys, so those can be skipped.
    else if( 1 == iindex->field("PRIMARY_KEY")->toInt() ) {
      continue;
    }
    // Is this an index that we've already done?  If so, it can be skipped.
    else if( indicesHandled.contains( *(iindex->field("INDEX_NAME")) ) ) {
      continue;
    } else {
      indexStr = "";
      indexName = *(iindex->field("INDEX_NAME"));
      indicesHandled.append( indexName );
      indexIsUnique = (1 == iindex->field("UNIQUE")->toInt() );

      // Begin the inner loop to assemble the column names.
      index = indices->fetchArrayFirst();
      while( NULL != index ) {

        if( ( 0 == index->field("TABLE_NAME")->compare( tableName ) ) && ( 0 == index->field("INDEX_NAME")->compare( indexName ) ) ) {
          if( indexStr.isEmpty() )
            indexStr = indexStr.append( CSqlDatabase::delimitedName( *(index->field("COLUMN_NAME")), dbFormat ) );
          else
            indexStr = indexStr.append( QString( ", %1" ).arg( CSqlDatabase::delimitedName( *(index->field("COLUMN_NAME")), dbFormat ) ) );
        }

        index = indices->fetchArrayNext();
      }

      ++nIndices;

      // Write the index to the stream.
      if( 1 == nIndices ) {
        *stream << "-- Other indices for table " << CSqlDatabase::delimitedName( tableName, dbFormat ) << endl;
      }

      if( CSqlDatabase::DBPostgres == dbFormat ) {
        if( indexIsUnique ) {
          *stream << QString( "CREATE UNIQUE INDEX %1 ON %2 ( %3 );" ).arg( CSqlDatabase::delimitedName( indexName, dbFormat ) ).arg( CSqlDatabase::delimitedName( tableName, dbFormat ) ).arg( indexStr ) << endl;
        } else {
          *stream << QString( "CREATE INDEX %1 ON %2 ( %3 );" ).arg( CSqlDatabase::delimitedName( indexName, dbFormat ) ).arg( CSqlDatabase::delimitedName( tableName, dbFormat ) ).arg( indexStr ) << endl;
        }
      }
      else {
        *stream << "-- Indices are not yet generated for this database type." << endl;
      }
    }
  }

  if( 0 == nIndices ) {
    *stream << "-- There are no indices for table " << CSqlDatabase::delimitedName( tableName, dbFormat ) << endl << endl;
  }
  else {
    *stream << endl << endl;
  }

  delete primaryKeys;
  delete indices;
}


bool dumpTableSql( QString tableName, CSqlDatabase* db, QTextStream* stream, const int dbFormat, const bool includeData, const int dataOnly ) {
  QStringList* list;
  CFieldInfoList* fieldInfoList;
  CSqlFieldInfo* fi;
  CSqlFieldInfo* lastFi;
  int j;
  bool result;
  QString comments = "";

  if( NULL == stream ) {
    //qDebug() << "No stream in dumpTableSql()";
    return true;
  }

  // Check that the table exists
  //----------------------------
  list = db->newTableList();

  if( !( list->contains( tableName ) ) ) {
    *stream << "ERROR: Table '" << tableName << "' doesn't exist" << endl << endl << flush;
    delete list;
    return( false );
  }

  // Get field information
  //----------------------
  fieldInfoList = db->newFieldInfoList( tableName, &result );

  if( result ) {
    if( !dataOnly ) {
      // Create the table structure
      //---------------------------
      *stream << "CREATE TABLE " <<  CSqlDatabase::delimitedName( tableName, dbFormat ) << " (" << endl;
  
      lastFi = fieldInfoList->last();
  
      for( j = 0; j < fieldInfoList->count(); ++j ) {
        fi = fieldInfoList->at(j);
        *stream
          << "  " << CSqlDatabase::delimitedName( fi->fieldName(), dbFormat )
          << " " << convertTypeDescr( fi, dbFormat )
        ;

        if( 0 < fi->fieldDescr().length() ) {
          switch( dbFormat ) {
            case CSqlDatabase::DBMySQL:
              *stream << " COMMENT " << CSqlDatabase::sqlQuote( fi->fieldDescr(), dbFormat );
              break;
            case CSqlDatabase::DBPostgres:
              comments = comments.append(
                QString( "COMMENT ON COLUMN %1.%2 IS %3;\n" )
                  .arg( CSqlDatabase::delimitedName( tableName, dbFormat ) )
                  .arg(  CSqlDatabase::delimitedName( fi->fieldName(), dbFormat ) )
                  .arg( CSqlDatabase::sqlQuote( fi->fieldDescr(), dbFormat ) )
              );
            default:
              break;
          }
        }

        if( fi != lastFi )
          *stream << "," << endl;
        else
          *stream << endl;
      }
      *stream << ");" << endl << endl;


      // Dump table comments (for Postgres)
      //-----------------------------------
      if( CSqlDatabase::DBPostgres == dbFormat ) {
        *stream << "-- Comments on columns:" << endl;
        *stream << comments << endl;
      }
    }

    // Dump the data 
    //--------------
    if( includeData ) {

      //qDebug() << "I should be dumping...";

      switch( dbFormat ) {
        case CSqlDatabase::DBMySQL:
          // fall through
        case CSqlDatabase::DBPostgres:
          dumpDataMySqlOrPostgres( tableName, db, stream, fieldInfoList, dbFormat );
          break;
        case CSqlDatabase::DBMSAccess:
          dumpDataAccess( tableName, db, stream, fieldInfoList );
          break;
        default:
          cout << "Database dump not supported for the selected format." << endl << flush;
          break;
      }
    }
  }

  *stream << endl;
  *stream << flush;

  delete fieldInfoList;
  delete list;

  return( result );
}



bool dumpSql( CSqlDatabase* db, QTextStream* stream, const int dbFormat, const bool includeData, const bool dataOnly ) {
  int i;
  bool result = false;
  CSqlResult* keys = new CSqlResult( "", db );
  QStringList* tableList = db->newTableList();
  CSqlRow* key;
  QString keyStr;
  int fkCounter;
  
  if( NULL == stream ) {
    //qDebug() << "No stream in dumpSql()";
    delete keys;
    return true;
  }

  // Dump table definitions and/or data
  //-----------------------------------
  for( i = 0; i < tableList->count(); ++i ) {
    result = dumpTableSql( (tableList->at(i)), db, stream, dbFormat, includeData, dataOnly );
    //stream << endl;
    if( false == result ) 
      break;
  }

  // Dump primary keys, etc.
  //------------------------
  if( result && !dataOnly ){
    // Dump primary keys, and indices
    // (and maybe some day, other constraints)
    *stream << "-- Table keys and constraints" << endl;
    for( i = 0; i < tableList->count(); ++i ){
      dumpTableConstraintsSql( tableList->at(i), db, stream, dbFormat );
    }


    // Dump foreign keys
    keys->foreignKeys();
    
    result = keys->success();
  
    if( result ) {
      *stream << "-- Foreign keys" << endl;

      key = keys->fetchArrayFirst();
      fkCounter = 0;
      
      while( NULL != key ) {
        keyStr.clear();

        // Access and MySQL keys are generated the same way, using ` as "quote delimiter"
        // for table/field names.  Postgres uses " instead of `.  Otherwise, the format is the same.
        switch( dbFormat ) {
          // Fall through for the first three.
          case CSqlDatabase::DBMSAccess:
          case CSqlDatabase::DBMySQL:
          case CSqlDatabase::DBPostgres:
            keyStr.append( QString( "ALTER TABLE %1\n" ).arg( CSqlDatabase::delimitedName( *(key->field( "FK_TABLE_NAME" )), dbFormat ) ) );

            keyStr.append(
              QString( "  ADD CONSTRAINT %1 FOREIGN KEY ( " ).arg(
                CSqlDatabase::delimitedName( QString( "%1_%2" ).arg( *(key->field("FK_NAME")) ).arg( fkCounter ), dbFormat )
              )
            );

            keyStr.append( QString(    "%1 )\n" ).arg( CSqlDatabase::delimitedName( *(key->field( "FK_COLUMN_NAME" )), dbFormat ) ) );
            keyStr.append( QString( "  REFERENCES %1 ( " ).arg( CSqlDatabase::delimitedName( *(key->field( "PK_TABLE_NAME" )), dbFormat ) ) );
            keyStr.append( QString(     "%1 );\n\n" ).arg( CSqlDatabase::delimitedName( *(key->field( "PK_COLUMN_NAME" )), dbFormat ) ) );
            break;
          default:
            cout << "Database format is not supported for SQL dumps." << endl << flush;
            break;
        }

        *stream << keyStr;
       
        ++fkCounter;
        key = keys->fetchArrayNext(); 
      }
    }
  }

  // Clean up
  //---------
  *stream << flush;

  delete keys;

  delete tableList;

  return result;
}



bool describeTable( QString tableName, CSqlDatabase* db, const bool isSystemTable, QTextStream* stream ) {
  CSqlFieldInfo* fi;
  TIntArray* sizeArray;
  CFieldInfoList* fieldInfoList;
  QStringList* list;
  QString tableComment;
  int maxNameLength = 5; // length of "Field"
  int maxFieldLength = 4; // length of "Type"
  int maxDescrLength = 11; // length of "Description"
  bool result;
  int i;

  QString s, t;

  if( NULL == stream ) {
    //qDebug() << "No stream in describeTable()";
    return true;
  }

  if( isSystemTable ) {
    list = db->newSystemTableList();
  }
  else {
    list = db->newTableList();
  }

  if( !( list->contains( tableName ) ) ) {
    *stream << "ERROR: Table '" << tableName << "' doesn't exist" << endl << endl << flush;
    delete list;
    return( false );
  }

  fieldInfoList = db->newFieldInfoList( tableName, &result );

  if( result ) {
    // Loop through the list once to determine max length of each column
    for( i = 0; i < fieldInfoList->count(); ++i ) {
      fi = fieldInfoList->at(i);
      if( fi->fieldName().length() > maxNameLength ) maxNameLength = fi->fieldName().length();
      if( fi->fieldTypeDescr().length() > maxFieldLength ) maxFieldLength = fi->fieldTypeDescr().length();
      if( fi->fieldDescr().length() > maxDescrLength ) maxDescrLength = fi->fieldDescr().length();
    }

    if( 60 < maxDescrLength ) maxDescrLength = 60;

    sizeArray = new TIntArray();
    sizeArray->append( maxNameLength );
    sizeArray->append( maxFieldLength );
    sizeArray->append( maxDescrLength );

    //tableComment = db->tableDescr( tableName );

    //if( 0 < tableComment.length() ) cout << tableComment << endl << flush;

    *stream << header( sizeArray )  << endl;
    *stream << "|" << column( "Field", maxNameLength ) << "|" << column( "Type", maxFieldLength ) << "|" << column( "Description", maxDescrLength ) << "|" << endl;
    *stream << header( sizeArray )  << endl;

    for( i = 0; i < fieldInfoList->count(); ++i ) {
      fi = fieldInfoList->at(i);
      *stream << "|" << column( fi->fieldName(), maxNameLength ) << "|" << column( fi->fieldTypeDescr(), maxFieldLength ) << "|"  << column( fi->fieldDescr(), maxDescrLength ) << "|" << endl;
    }

    *stream << header( sizeArray )  << endl;
    *stream << fieldInfoList->count() << " rows in set" << endl;
    *stream << endl;

    delete sizeArray;
  }

  *stream << flush;

  delete fieldInfoList;
  delete list;

  return( result );
}



bool describeAllTables( CSqlDatabase* db, QTextStream* stream ) {
  QString str;
  bool result = true;
  int i;

  if( NULL == stream ) {
    //qDebug() << "No stream in describeAllTables()";
    return true;
  }

  QStringList* tableList = db->newTableList();

  for( i = 0; i < tableList->count(); ++i ) {
    str = tableList->at(i);
    *stream << "jetsql> describe " << str << ";" << endl;
    if( !( describeTable( QString( str ), db, false, stream ) ) ) result = false;
  }

  delete tableList;
  return( result );
}



QString* newListLabel( QString label, int desiredLen ) {
  int i;
  int diff = desiredLen - label.length();
  QString* lbl = new QString( label );
  for( i = 0; diff > i; ++i ) lbl->prepend( " " );
  return lbl;
}



void printListFormat( CSqlResult* res, QTextStream* stream ) {
  CSqlRow* row;
  int rowCount;
  unsigned int i;
  QString* str;
  int maxNameLen = 0;
  CQStringList* labelList = new CQStringList();
  labelList->setAutoDelete( true );

  if( NULL == stream ) {
    //qDebug() << "No stream in printListFormat()";
    delete labelList;
    return;
  }

  // Determine the max field name length.
  for( i = 0; res->fieldCount() > i; ++i ) {
    if( res->fieldName( i ).length() > maxNameLen ) maxNameLen = res->fieldName( i ).length();
  }

  // Loop over fields again, creating and storing the field labels.
  for( i = 0; res->fieldCount() > i; ++i ) {
    str = newListLabel( res->fieldName( i ), maxNameLen );
    labelList->append( str );
  }

  // Loop over the records, writing the information to the console.
  rowCount = 1;
  for( row = res->fetchArrayFirst(); NULL != row; row = res->fetchArrayNext() ) {
    *stream << "*************************** " << rowCount << ". row ***************************" << endl;
    for( i = 0; res->fieldCount() > i; ++i ) {
      if( row->field(i)->isNull() ) {
        *stream <<  *(labelList->at( i ))  <<  ": " << "NULL" << endl;
      }
      else {
        *stream <<  *(labelList->at( i )) <<  ": " << *(row->field( i )) << endl;
      }
    }
    ++rowCount;
  }
  *stream << endl;
  *stream << flush;

  delete labelList;
}



void printTableFormat( CSqlResult* res, QTextStream* stream ) {
  CSqlRow* row;
  unsigned int i;
  TIntArray* arr;
  
  if( NULL == stream ) {
    //qDebug() << "No stream in printTableFormat()";
    return;
  }

  // Create and clear the field size array
  arr = new TIntArray();
  for( i = 0; i < res->fieldCount(); ++i ) {
    arr->append( 0 );
  }

  // Loop through the result set once to determine max sizes
  for( row = res->fetchArrayFirst(); NULL != row; row = res->fetchArrayNext() ) {
    for( i = 0; res->fieldCount() > i; ++i ) {
      if( row->field( i )->length() > arr->at(i) )
        arr->replace( i, row->field(i)->length() );
    }
  }

  // Don't forget the field names in the header
  for( i = 0; res->fieldCount() > i; ++i ) {
    if( res->fieldName( i ).length() > arr->at(i) )
      arr->replace( i, res->fieldName( i ).length() );
  }

  // Start writing!
  *stream << header( arr ) << endl;

  for( i = 0; res->fieldCount() > i; ++i ) {
    *stream << "|" << column( res->fieldName( i ), arr->at(i) );
  }
  *stream << "|" << endl;
  *stream << header( arr ) << endl;

  for( row = res->fetchArrayFirst(); NULL != row; row = res->fetchArrayNext() ) {
    for( i = 0; res->fieldCount() > i; ++i ) {
      if( row->field(i)->isNull() ) {
        *stream << "|" << column( "NULL", arr->at(i) );
      }
      else {
        *stream << "|" << column( *(row->field(i)), arr->at(i) );
      }
    }
    *stream << "|" << endl;
  }
  *stream << header( arr ) << endl;
  *stream << flush;

  delete arr;
}



bool showKeys( CSqlDatabase* db, QTextStream* stream, const int format, const int keyType ) {
  bool result;
  CSqlResult* res = new CSqlResult( "", db );  

  switch( keyType ) {
    case CSqlDatabase::DBKeyTypePrimary:
      res->primaryKeys();
      break;
    case CSqlDatabase::DBKeyTypeForeign:
      res->foreignKeys();
      break;
    case CSqlDatabase::DBKeyTypeIndex:
      res->indices();
      break;
  }
  
  result = res->success();

  if( result ) {
    //qDebug() << "Successful key query.";
    
    if( 0 == res->numRows() ) {
      *stream << endl << "Empty set" << endl << flush;
    }
    else {
      if( FmtList == format ) {
        //qDebug() << "Printing list format...";
        printListFormat( res, stream );
      }
      else {
        //qDebug() << "Printing table format...";
        printTableFormat( res, stream );
      }
      *stream << res->numRows() << " rows in set" << endl << flush;
    }
  }
  else {
    *stream << "Query failed: " << res->lastError() << endl << flush;
  }

  delete res;

  return( result );
} 



bool printRecords( SSqlCommand sqlCmd, CSqlDatabase* db, QTextStream* stream ) {
  bool result;
  CSqlResult* res = new CSqlResult( sqlCmd.cmdText, db );

  result = res->success();

  if( result ) {
    //qDebug() << "Successful query.";

    if( 0 == res->numRows() ) {
      *stream << endl << "Empty set" << endl << flush;
    }
    else {
      if( FmtList == sqlCmd.formatType ) {
        //qDebug() << "Printing list format...";
        printListFormat( res, stream );
      }
      else {
        //qDebug() << "Printing table format...";
        printTableFormat( res, stream );
      }
      *stream << res->numRows() << " rows in set" << endl << flush;
    }
  }
  else {
    *stream << "Query failed: " << res->lastError() << endl << flush;
  }

  delete res;

  return( result );
}



QString extractFileName( QString& cmd ) {
  QString result;
  QChar s;
  uint i, j;
  //bool inQuotes;

  cmd = cmd.trimmed();
  //qDebug() << cmd;

  result = "";
  j = cmd.length();
  for( i = 0; i < j; ++i ) {
    s = cmd.at(0);
    if( s.isSpace() ) {
      break;
    }
    else {
      result = result + s;
      cmd = cmd.right( cmd.length() - 1 );
    }
  }

  result = result.trimmed();
  cmd = cmd.trimmed();

  return result;
}



bool processSqlCommand( SSqlCommand cmd, CSqlDatabase* db, QTextStream* stream, bool silent ) {
  QString sqlCmd;
  
  QString adminCmd;
  QStringList adminCmdParts;
  bool tableFound;
  QString tableName;
  int outputFormat;
  bool includeData;
  bool dataOnly;
  int tmp;
  
  unsigned int rowsAffected;
  bool result = true;

  sqlCmd = cmd.cmdText.trimmed(); 
  //qDebug() << sqlCmd;

  if( isSqlComment( sqlCmd ) ) {
    result = true;
  }
  else {
    // Check for 'admin' functions
    //----------------------------
    // Use a white-space-simplified version of the command, but
    // don't mess with the original command, just in case the internal white space
    // is part of a query.
    adminCmd = sqlCmd.simplified();
    
    if( "sqldump" == adminCmd.left(7).toLower() ) {
      adminCmdParts = adminCmd.split( QRegExp( "\\s+" ) );  
      
      // Is there a table name included in the command?
      //-----------------------------------------------
      tmp  = adminCmdParts.indexOf( QRegExp( "^table$", Qt::CaseInsensitive ) ); 
      if( -1 != tmp ) { // There is a table.
        // Make sure that "table" isn't the last token in the command.
        // If it is, there is a problem.
        if( adminCmdParts.count() - 1 == tmp ) {
          if( !silent )
            *stream << "Bad SQLDUMP command: table name missing." << endl << flush;
          return false;  
        }
        else {
          tableFound = true;
          tableName = adminCmdParts.at( tmp + 1 );  
        }
      }
      else {
        tableFound = false;
        tableName = ""; 
      } 
    
      // What is the desired output format?
      //-----------------------------------
      if( -1 != adminCmdParts.indexOf( QRegExp( "^mysql$", Qt::CaseInsensitive ) ) ) {
        outputFormat = CSqlDatabase::DBMySQL;   
      }
      else if( -1 != adminCmdParts.indexOf( QRegExp( "^postgres$", Qt::CaseInsensitive ) ) ) {
        outputFormat = CSqlDatabase::DBPostgres;
      }
      else { // Default to MS Access
        outputFormat = CSqlDatabase::DBMSAccess;  
      }
      
      // Does the user want data, or just the table schema?
      //---------------------------------------------------
      if( -1 != adminCmdParts.indexOf( QRegExp( "^nodata$", Qt::CaseInsensitive ) ) ) {
        includeData = false;
      }
      else {
        includeData = true;
      }
      
      // Does the user want only data, without the table schema?
      //--------------------------------------------------------
      if( -1 != adminCmdParts.indexOf( QRegExp( "^dataonly$", Qt::CaseInsensitive ) ) ) {
        dataOnly = true;  
      }
      else {
        dataOnly = false;
      }
      
      // Does the user really know what he wants?
      //-----------------------------------------
      if( dataOnly && !includeData ) {
        if( !silent )
          *stream << "Bad SQLDUMP command: conflicting options." << endl << flush;
        return false;         
      }
      
      // Proceed with the command
      //-------------------------
      if( tableFound ) {
        if( !silent ) {
          result = dumpTableSql( tableName, db, stream, outputFormat, includeData, dataOnly );
          if( result ) {
            dumpTableConstraintsSql( tableName, db, stream, outputFormat );
          }
        }
      }
      else { // Dump all tables
        if( !silent ) {
          result = dumpSql( db, stream, outputFormat, includeData, dataOnly );
        }
      }
       
    }
    else if( "primary keys" == adminCmd.left(12).toLower() ) {
      if( !silent ) 
        result = showKeys( db, stream, cmd.formatType, CSqlDatabase::DBKeyTypePrimary );
    }
    else if( "foreign keys" == adminCmd.left(12).toLower() ) {
      if( !silent )
        result = showKeys( db, stream, cmd.formatType, CSqlDatabase::DBKeyTypeForeign );
    }
    else if( "indices" == adminCmd.left(7).toLower() ) {
      if( !silent )
        result = showKeys( db, stream, cmd.formatType, CSqlDatabase::DBKeyTypeIndex );
    }
    else if ( "describe tables" == adminCmd.left( 15 ).toLower() ) {
      if( !silent ) 
        result = describeAllTables( db, stream );
    }
    else if( "show tables" == adminCmd.left( 11 ).toLower() ) {
      if( !silent ) 
        result = showTables( db, stream );
    }
    else if( "show system tables" == adminCmd.left( 18 ).toLower() ) {
      if( !silent ) 
        result = showSystemTables( db, stream );
    }
    else if( "sysdescribe" == adminCmd.left( 11 ).toLower() ) {
      if( !silent ) 
        result = describeTable( extractTableName( adminCmd ), db, true, stream );
    }
    else if( "describe" == adminCmd.left( 8 ).toLower() ) {
      if( !silent ) 
        result = describeTable( extractTableName( adminCmd ), db, false, stream );
    }
    else if( "csvdump table" == adminCmd.left( 13 ).toLower() ) {
      if( !silent ) 
        result = dumpTableCsv( extractTableName( adminCmd ), db, stream );
    }
    else if( "help" == adminCmd.left( 4 ).toLower() ) {
      if ( !silent ) {
        showHelp();
        result = true;
      }
    }


    // Check for a typical query
    //--------------------------
    else if( CSqlQueryString::isSelect( sqlCmd ) ) {
      if( !silent ) 
        result = printRecords( cmd, db, stream );
    }
    else if( db->execute( sqlCmd, &rowsAffected ) ) {
      result = true;
      if( !silent ) {
        *stream << "Query was successful." << endl << flush;
  
        // FIX ME(?) Because of something in the bowels of ADO (I think), the max value returned by rowsAffected is
        // 32000 (a short) rather than 4 x 10^7 (an unsigned int).
        // I can find now way to fix this...
        *stream << "Records affected: " << rowsAffected << endl << flush;
      }
    }
    else {
      result = false;
      if( !silent )
        *stream << "Query failed: " << db->lastError() << endl << flush;
    }
  }

  return( result );
}



bool isFile( QString sqlCmd ) {
  if( sqlCmd.left( 5 ) == "FILE " ) {
    return true;
  }
  else {
    return false;
  }
}



CQStringList* newListFromSqlFile( QFile* sqlFile ) {
  QString contents = QString();
  QString line, line2;
  QString temp;
  QString* cmd;

  CQStringList* cmdList = new CQStringList;
  cmdList->setAutoDelete( true );

  if( sqlFile->open( QIODevice::ReadOnly ) ) {
    QTextStream ts ( sqlFile );

    while( !( ts.atEnd() ) ) {
      line = ts.readLine();
      line2 = line.trimmed();
      if( line2.left( 2 ) != "--" && line2.left( 1 ) != "#" ) {
        temp.append( line2 );

        if( ";" == temp.right( 1 ) || "\\G" == temp.right( 2 ) ) {
          cmd = new QString( temp );
          cmdList->append( cmd );
          temp = QString();
        }
        else {
          temp.append( " " );
        }
      }
    }
    sqlFile->close();
  }

  return cmdList;
}


// return true if an error occurs.
bool processFile( CSqlDatabase* db, QFile* sqlFile, QTextStream* stream, bool continueOnError, bool silent ) {
  QString* cmd;
  CQStringList* cmdList;
  SSqlCommand scmd;

  bool success;
  int successCount = 0;;
  int failureCount = 0;

  int i;

  // Read the file into the list...
  cmdList = newListFromSqlFile( sqlFile );
  //qDebug() << "Items in cmdList:" << cmdList->count();
  
  // ... and individually process the list items

  for( i = 0; i < cmdList->count(); ++i ) {
    cmd = cmdList->at(i);
    //qDebug() << cmd->trimmed();
    
    if( !silent )
      *stream << endl << "file> " << cmd->trimmed() << endl << flush;

    if( ";" == cmd->right(1) ) {
      scmd.cmdText = cmd->left( cmd->length() - 1 );
      scmd.formatType = FmtTable;
    }
    else if( "\\G" == cmd->right(2) ) {
      scmd.cmdText = cmd->left( cmd->length() - 2 );
      scmd.formatType = FmtList;
    }
    else {
      scmd.cmdText == *cmd;
      scmd.formatType = FmtTable;
    }

    if( !( isSqlComment( scmd.cmdText.trimmed() ) ) ) {
      success = processSqlCommand( scmd, db, stream, silent );  

      if( success ) {
        ++successCount;
      }
      else {
        ++failureCount;
      }

      if( !( continueOnError ) && !( success ) ) {
        if( !silent )
          *stream << endl << "Execution of commands in file '" << sqlFile->fileName() << "' interrupted." << endl << flush;
        break;
      }
    }
  }

  if( !silent ) {
    if( 0 == failureCount ) {
      *stream << endl << "Execution of commands in file '" << sqlFile->fileName() << "' complete." << endl;
    }
    *stream << "Successful queries: " << successCount << endl;
    *stream << "Failed queries: " << failureCount << endl << flush;
  }
  delete cmdList;
  
  return ( 0 != failureCount );
}


void processFileCommand( QString sqlCmd, CSqlDatabase* db, QTextStream* stream ) {
  QStringList parts;
  QString fn;
  bool continueOnError = false;
  QFile* sqlFile;
       
  // sqlCmd will have three parts:
  // "FILE " - a 5-character tag, indicating that this command is really a file.
  // filename - the name of the file containing the commands to execute.
  // "CONTINUE" - (optional) used where the user wants to count the number of commands that don't execute, rather than stopping upon an error.

  // Split sqlCommand to determine what's in it...
  parts = sqlCmd.split( QRegExp( "\\s+" ) );
  fn = parts[1].trimmed();

  if( 3 == parts.count() ) {
    continueOnError = ( "continue" == parts[2].trimmed().toLower() );
  }
  
  sqlFile = new QFile( fn );
  if( sqlFile->exists() ) {
    processFile( db, sqlFile, stream, continueOnError, false );
  }
  else {
    *stream << "File " << fn << " doesn't exist." << endl << flush;
  }
  
  delete sqlFile;
}


void showVersion( void ) {
  cout << "JetSQLConsole " << VERSION_NUMBER << endl;
  cout << "Copyright (C) 2003 - 2014 Aaron Reeves." << endl;
  cout << "This is free software, released under the terms of the GNU General Public License." << endl;
  cout << "Please see the source or the following URL for copying conditions." << endl;
  cout << "JetSQLConsole home page: <http://www.aaronreeves.com/jetsqlconsole>" << endl;
  cout << endl << flush;
}


void showHelp( void ) {
  showVersion();
  cout << "Command line arguments:" << endl;
  cout << "  --help, -h, -?:   Display this help message." << endl;
  cout << "  --version, -v:    Display version information." << endl;
  cout << "  --database <filename>, -d <filename>:" << endl;
  cout << "                    If --database is the only switch provided, the" << endl;
  cout << "                    specified database will be opened for interactive" << endl;
  cout << "                    use.  If --database and --file are both present," << endl;
  cout << "                    SQL commands from the file will be run without" << endl;
  cout << "                    further user interaction." << endl;
  cout << "  --file <filename>, -f <filename>:" << endl;
  cout << "                    Specifies the name of a plain-text file containing" << endl;
  cout << "                    SQL commands to be executed against a database." << endl;
  cout << "                    Used in combination with --database." << endl;
  cout << "  --dbf, -dbf       Open a DBase file, instead of a Microsoft Access file." << endl;
  cout << "  --output <filename>, -o <filename>:" << endl;
  cout << "                    Specifies the name of a plain-text file to write" << endl;
  cout << "                    with results from SQL commands executed against a" << endl;
  cout << "                    database.  If --database and --file are not given," << endl;
  cout << "                    this switch is ignored." << endl;
  cout << "  --continue, -c:   If preseent, execution of SQL commands " << endl;
  cout << "                    in the file will continue even if an" << endl;
  cout << "                    error occurs.  Otherwise, execution will stop on" << endl;
  cout << "                    an error." << endl;
  cout << "  --silent, -s:     Run without writing any output to the console." << endl;
  cout << "                    If --database, --file, and --silent are all given," << endl;
  cout << "                    output will not be written to the console." << endl;
  cout << "                    Otherwise, this switch is ignored." << endl;
  cout << endl;
  cout << "Interactive mode commands:" << endl;
  cout << "  show tables:      Displays a list of tables in the database, excluding" << endl;
  cout << "                    Microsoft Access system tables." << endl;
  cout << "  show system tables:" << endl;
  cout << "                    Displays a list of  Microsoft Access system tables" << endl;
  cout << "                    in the database." << endl;
  cout << "  describe <tablename>:" << endl;
  cout << "                    Displays a description of the indicated table." << endl;
  cout << "                    Excludes system tables." << endl;
  cout << "  describe tables:  Describes all tables, excluding system tables." << endl;
  cout << "  sysdescribe <tablename>:" << endl;
  cout << "                    Displays a description of the indicated system table." << endl;
  cout << "  keys:             Displays a list of all keys/constraints in the database." << endl;
  cout << endl;
  cout << "  open outfile <filename>:" << endl;
  cout << "                    Open a file and direct output (e.g., results of queries)" << endl;
  cout << "                    to it." << endl;
  cout << "  close outfile:    Stop directing output to a previously opened file and" << endl;
  cout << "                    close it." << endl;
  cout << "  sqldump [nodata] [mysql|postgres] [tablename]:" << endl;
  cout << "                    Generates a complete set of DDL instructions to" << endl;
  cout << "                    regenerate and (optionally) populate the database." << endl;
  cout << "                    If the option 'nodata' is provided, the generated" << endl;
  cout << "                    script will only reproduce the structure of the" << endl;
  cout << "                    database.  Options 'mysql' or 'postgres' produce" << endl;
  cout << "                    DDL/SQL statements compatible with MySQL or PostgreSQL," << endl;
  cout << "                    respectively. Otherwise, the generated script is" << endl;
  cout << "                    suitable for use with Microsoft Access." << endl;
  cout << "                    If option 'tablename' is provided, DDL/SQL for only the" << endl;
  cout << "                    indicated table is generated." << endl;
  cout << "  csvdump table <tablename>:" << endl;
  cout << "                    Writes the contents of the indicated table in comma-" << endl;
  cout << "                    separated format." << endl;
  cout << endl;
  cout << "Exit codes:" << endl;
  cout << "  0: Operation was successful." << endl;
  cout << "  1: Database file not specified." << endl;
  cout << "  2: Database file does not exist or could not be opened." << endl;
  cout << "  3: SQL command file not specified." << endl;
  cout << "  4: SQL command file does not exist or could not be opened." << endl;
  cout << "  5: SQL commands resulted in an error." << endl;
  cout << "  6: Output file could not be created or written." << endl;
  cout << endl << flush;
}


void handleSqlFileFromCommandLine( CCmdLine* cmdLine, int& exitCode ) {
  bool silent = false;
  bool continueOnError = false;
  QString dbPath, sqlPath;
  QFile* sqlFile = NULL; 
  CSqlDatabase* db = NULL;
  bool errorOccurred = false;
  bool useOutputFile = false;
  QString outputPath;  
  QFile* outputFile = NULL;
  QTextStream* outputStream = NULL;
  
  // Other optional switches include --silent (-s) and --continue (-c).   
  silent = ( cmdLine->HasSwitch( "-s" ) || cmdLine->HasSwitch( "--silent" ) );
  continueOnError = ( cmdLine->HasSwitch( "-c" ) || cmdLine->HasSwitch( "--continue" ) );
  
  
  // Did the user specify a legitimate database file?
  //-------------------------------------------------
  if( cmdLine->HasSwitch( "-d" ) ) {
    dbPath = cmdLine->GetSafeArgument( "-d", 0, "" );
  }
  else if( cmdLine->HasSwitch( "--database" ) ) {
    dbPath = cmdLine->GetSafeArgument( "--database", 0, "" );
  }
  else {
    dbPath = "";
  }
    
  // Is the path specified?
  if( 0 == dbPath.trimmed().length() ) {
    if( !silent ) {
      cout << "JetSQLConsole " << VERSION_NUMBER << endl;
      cout << "Missing database file name.  Use '-h' for help." << endl << endl << flush;  
    } 
    exitCode = 1;
    return; 
  }
  
  // Does the specified file exist?
  if( !( QFile::exists( dbPath ) ) ) {
    if( !silent ) {
      cout << "JetSQLConsole " << VERSION_NUMBER << endl;
      cout << "Database '" << dbPath << "' does not exist." << endl << endl << flush;  
    } 
    exitCode = 2;
    return;       
  }
      
  // Can the database file be opened?
  db = new CSqlDatabase();
  db->setType( CSqlDatabase::DBMSAccess );
  db->setAction( CSqlDatabase::DBOpen );
  db->setName( dbPath );
  db->open();
  dbPath = ""; // Reset this once the file is open to prevent an infinite loop.

  if( !( db->isOpen() ) ) {
    if( !silent ) {
      cout << "JetSQLConsole " << VERSION_NUMBER << endl;
      cout << "Database '" << dbPath << "' could not be opened." << endl << endl << flush;  
    } 
    exitCode = 2;
    db->close();
    delete db;
    return; 
  }
  

  // Did the user specify a legitimate SQL command file?
  //----------------------------------------------------
  if( cmdLine->HasSwitch( "-f" ) ) {
    sqlPath = cmdLine->GetSafeArgument( "-f", 0, "" );
  }
  else if( cmdLine->HasSwitch( "--file" ) ) {
    sqlPath = cmdLine->GetSafeArgument( "--file", 0, "" );
  }
  else {
    sqlPath = ""; 
  }
  
  
  // Is the path specified?
  if( 0 == sqlPath.trimmed().length() ) {
    if( !silent ) {
      cout << "JetSQLConsole " << VERSION_NUMBER << endl;
      cout << "Missing SQL command file name.  Use '-h' for help." << endl << endl << flush;  
    } 
    exitCode = 3;
    return; 
  }
  
  // Does the specified file exist?
  if( !( QFile::exists( sqlPath ) ) ) {
    if( !silent ) {
      cout << "JetSQLConsole " << VERSION_NUMBER << endl;
      cout << "File '" << sqlPath << "' does not exist." << endl << endl << flush;  
    } 
    exitCode = 4;
    return;       
  }
    
  // Can the file be opened?
  sqlFile = new QFile( sqlPath );
  if( !( sqlFile->open( QIODevice::ReadOnly ) ) ) {
    if( !silent ) {
      cout << "JetSQLConsole " << VERSION_NUMBER << endl;
      cout << "File '" << sqlPath << "' could not be opened." << endl << endl << flush;  
    } 
    exitCode = 4;
    db->close();
    delete db;
    delete sqlFile;
    return;   
  }
  sqlFile->close();
  
  
  // Does the user want an output file?  If so, is it properly specified?
  //---------------------------------------------------------------------
  if( cmdLine->HasSwitch( "-o" ) ) {
    outputPath = cmdLine->GetSafeArgument( "-o", 0, "" );
    useOutputFile = true;
  }
  else if( cmdLine->HasSwitch( "--output" ) ) {
    sqlPath = cmdLine->GetSafeArgument( "--output", 0, "" );
    useOutputFile = true;
  }
  else {
    sqlPath = ""; 
    useOutputFile = false;
  }    
  
  
  if( useOutputFile ) {
    if( 0 == outputPath.length() ) {
      if( !silent ) {
        cout << "JetSQLConsole " << VERSION_NUMBER << endl;
        cout << " Ouptut file name not specified." << endl << endl << flush; 
      } 
      exitCode = 6;
      db->close();
      delete db;
      delete sqlFile;
      return;
    }
    
    outputFile = new QFile( outputPath );
    if( !( outputFile->open( QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text ) ) ) {
      if( !silent ) {
        cout << "JetSQLConsole " << VERSION_NUMBER << endl;
        cout << "Output file '" << outputPath << "' could not be opened." << endl << endl << flush;  
      } 
      exitCode = 4;
      db->close();
      delete db;
      delete sqlFile;
      delete outputFile;     
      return;
    }
  
    // If we get this far, create the output stream from the file 
    outputStream = new QTextStream( outputFile ); 
  }
  else { // There is no output file: write output to stdout.
    outputStream = new QTextStream( stdout, QIODevice::WriteOnly );
  }
  
  // If we get this far, attempt to execute the commands from the file.
  //-------------------------------------------------------------------
  errorOccurred = processFile( db, sqlFile, outputStream,  continueOnError, silent ); 
  
  if( errorOccurred ) {
    exitCode = 5;
  }
  else {
    exitCode = 0;
  }
  
  db->close();
  delete db;
  delete sqlFile;

  if( NULL != outputFile ) {
    outputFile->close();
  }
  delete outputStream;
  
  if( NULL != outputFile ) {
    delete outputFile;
  }
  
  return;
}


bool handleSwitch( CCmdLine* cmdLine, QString& dbPath, int& exitCode ) {
  bool dbSwitchPresent, fileSwitchPresent;
  
  dbSwitchPresent = ( cmdLine->HasSwitch( "-d" ) || cmdLine->HasSwitch( "--database" ) );
  fileSwitchPresent = ( cmdLine->HasSwitch( "-f" ) || cmdLine->HasSwitch( "-filename" ) ); 
  
  if( cmdLine->HasSwitch( "-h" ) || cmdLine->HasSwitch( "--help" ) || cmdLine->HasSwitch( "-?" ) ) {
     showHelp();
     return true;
  }
  else if( cmdLine->HasSwitch( "-v" ) || cmdLine->HasSwitch( "--version" ) ) {
    showVersion();
    return true;
  }
  else if( dbSwitchPresent && fileSwitchPresent ) {
    handleSqlFileFromCommandLine( cmdLine, exitCode );
    return true;
  }
  else if( dbSwitchPresent && !( fileSwitchPresent ) ) {
    if( cmdLine->HasSwitch( "-d" ) ) {
      dbPath = cmdLine->GetSafeArgument( "-d", 0, "" );
    }
    else if( cmdLine->HasSwitch( "--database" ) ) {
      dbPath = cmdLine->GetSafeArgument( "--database", 0, "" );
    }
    return false;  
  }
  else {
    cout << "JetSQLConsole " << VERSION_NUMBER << endl;
    cout << "Incorrect command line switch(es).  Use '-h' for help." << endl << endl << flush;
    return true;
  }
}



int main( int argc, char **argv ) {
  SSqlCommand sqlCmd;
  QString dbPath = "";
  bool exitAfterSwitch = false;
  CSqlDatabase* db = NULL;

  QString fileName;
  QFile* file = NULL;
  QTextStream* stream;
  QTextStream* fout = NULL;
  QTextStream stdCout( stdout, QIODevice::WriteOnly );

  bool isDbfFile;
  
  int exitCode = 0;
  int nSwitches;
  
  CCmdLine* cmdLine = new CCmdLine( argc, argv );

  nSwitches = cmdLine->getSwitchCount();

  isDbfFile = cmdLine->HasSwitch( "-dbf" ) || cmdLine->HasSwitch( "--dbf" );

  if ( 0 < nSwitches ) { // At least one switch is present on the command line.
    // Do something with the it/them.
    //qDebug() << "Handling switches...";
    exitAfterSwitch = handleSwitch( cmdLine, dbPath, exitCode );

    //qDebug() << "Done handling switches.";
    if( exitAfterSwitch ) {
      delete cmdLine;
      qDebug() << "Everything should be done!";
      qDebug() << exitCode;
      return exitCode;
    }
  }

  // Carry on as usual after handling the switch
  cout << "JetSQLConsole " << VERSION_NUMBER << endl << flush;

  while( true ) {
    if( 0 == dbPath.length() ) dbPath = getDbPath( isDbfFile );
    if( dbPath == "quit" || dbPath == "exit" ) { // user wants to quit
      if( !( NULL == db ) ) {
        db->close();
        delete db;
        db = NULL;
      }
      delete cmdLine;
      cout << "Bye" << endl << endl << flush;
      return 0;
    }
    else {
      db = new CSqlDatabase();
      if( isDbfFile )
        db->setType( CSqlDatabase::DBDbf );
      else
        db->setType( CSqlDatabase::DBMSAccess );

      db->setAction( CSqlDatabase::DBOpen );
      db->setName( dbPath );
      db->open();
      dbPath = ""; // Reset this once the file is open to prevent an infinite loop.

      if(  db->isOpen() ) {
        cout << "Database is open." << endl << flush;
        // Now move on with the application.
      }
      else {
        cout << "Database didn't open." << endl << flush;
        continue; // Go back to the top of the outer loop
      }
    }

    stream = &stdCout;

    while( true ) {
      sqlCmd = getSqlCmd();

      if( "quit" == sqlCmd.cmdText.toLower().left( 4 ) || "exit" == sqlCmd.cmdText.toLower().left( 4 ) ) {
        cout << "Bye" << endl << endl << flush;
        db->close();
        delete db;
        db = NULL;
        delete cmdLine;
        return 0;
      }
      else if( "version" == sqlCmd.cmdText.toLower().left( 7 ) ) {
        showVersion();
      }
      else if( "help" == sqlCmd.cmdText.toLower().left( 4 ) ) {
        showHelp();
      }
      else if( "open outfile" == sqlCmd.cmdText.toLower().left( 12 ) ) {
        fileName = sqlCmd.cmdText.right( sqlCmd.cmdText.length() - 12 ).trimmed();
        file = new QFile( fileName );
        if( !file->open( QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text ) ) {
          cout << "Cannot open file " << fileName.toLatin1().data() << flush;
          cout << endl << flush;
          delete file;
        }
        else {
          fout = new QTextStream( file );
          stream = fout;
        }
      }
      else if( "close outfile" == sqlCmd.cmdText.toLower().left( 13 ) ) {
        file->close();
        delete fout;
        delete file;
        stream = &stdCout;
      }
      else if( "close" == sqlCmd.cmdText.toLower().left( 5 ) ) {
        cout << "Closing " << db->name().toLatin1().data() << endl << endl << flush;
        db->close();
        delete db;
        db = NULL;
        break; // Go back to the outermost loop, and select a new database to open.
      }
      else if( isFile( sqlCmd.cmdText ) ){
        processFileCommand( sqlCmd.cmdText, db, stream );
      }
      else {
        processSqlCommand( sqlCmd, db, stream, false );
      }

      *stream << flush;
    }
  }

  delete cmdLine;
  return 0;
}



