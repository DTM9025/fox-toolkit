/********************************************************************************
*                                                                               *
*                           S e t t i n g s   C l a s s                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: FXSettings.cpp,v 1.16.4.1 2002/11/13 15:14:07 fox Exp $                      *
********************************************************************************/
#ifdef HAVE_VSSCANF
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXStringDict.h"
#include "FXSettings.h"

/*
  Notes:

  - Format for settings database file:

    [Section Key]
    EntryKey=string-with-no-spaces
    EntryKey="string\nwith a\nnewline in it\n"
    EntryKey="string with spaces and \"embedded\" in it"

  - EntryKey may is of the form "ali baba", "ali-baba", "ali_baba", or "ali.baba".

  - Leading/trailing spaces are NOT part of the EntryKey.

  - FXSectionDict should go; FXSettings should simply derive from FXDict.

*/

#define MAXBUFFER 2000
#define MAXNAME   200
#define MAXVALUE  2000


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXSettings,FXDict,NULL,0)


// Make registry object
FXSettings::FXSettings(){
  modified=FALSE;
  }


// Create data
void *FXSettings::createData(const void*){
  return new FXStringDict;
  }


// Delete data
void FXSettings::deleteData(void* ptr){
  delete ((FXStringDict*)ptr);
  }


// Parse filename
FXbool FXSettings::parseFile(const FXString& filename,FXbool mark){
  FXchar buffer[MAXBUFFER],name[MAXNAME],value[MAXVALUE];
  FXStringDict *group;
  FXchar *ptr,c;
  FXint lineno;
  FILE *file;
  FXint len;
  file=fopen(filename.text(),"r");
  if(file){
    FXTRACE((100,"Reading settings file: %s\n",filename.text()));
    group=NULL;
    lineno=1;
    while(fgets(buffer,MAXBUFFER,file)!=NULL){

      // Parse buffer
      ptr=buffer;

      // Skip leading spaces
      while(*ptr && isspace((FXuchar)*ptr)) ptr++;

      // Test for comments
      if(*ptr=='#' || *ptr==';' || *ptr=='\0') goto next;

      // Parse section name
      if(*ptr=='['){
        ptr++;
        len=0;
        while((c=*ptr)!='\0' && c!=']'){
          if((FXuchar)c<' '){
            fxwarning("%s:%d: illegal section name.\n",filename.text(),lineno);
            goto next;
            }
          if(len>=MAXNAME){
            fxwarning("%s:%d: section name too long.\n",filename.text(),lineno);
            goto next;
            }
          name[len]=c;
          len++;
          ptr++;
          }
        name[len]='\0';

        // Add new section dict
        group=insert(name);
        }

      // Parse key name
      else{

        // Should have a group
        if(!group){
          fxwarning("%s:%d: settings entry should follow a section.\n",filename.text(),lineno);
          goto next;
          }

        // Transfer key, checking validity
        len=0;
        while((c=*ptr)!='\0' && c!='='){
          if((FXuchar)c<' '){
            fxwarning("%s:%d: illegal key name.\n",filename.text(),lineno);
            goto next;
            }
          if(len>=MAXNAME-1){
            fxwarning("%s:%d: key name too long.\n",filename.text(),lineno);
            goto next;
            }
          name[len]=c;
          len++;
          ptr++;
          }

        // Remove trailing spaces from key
        while(len && name[len-1]==' ') len--;
        name[len]='\0';

        // Should be a '='
        if(*ptr++!='='){
          fxwarning("%s:%d: expected '=' to follow key.\n",filename.text(),lineno);
          goto next;
          }

        // Skip more spaces
        while(*ptr && isspace((FXuchar)*ptr)) ptr++;

        // Parse value
        if(!parseValue(value,ptr)){
          fxwarning("%s:%d: error parsing value.\n",filename.text(),lineno);
          goto next;
          }

        // Add entry to current section
        group->replace(name,value,mark);
        }
next: lineno++;
      }
    fclose(file);
    return TRUE;
    }
  return FALSE;
  }


// Parse value
FXbool FXSettings::parseValue(FXchar* value,const FXchar* buffer){
  register const FXchar *ptr=buffer;
  register FXchar *out=value;
  unsigned int v1,v2,h,l;

  // Was quoted string; copy verbatim
  if(*ptr=='"'){
    ptr++;
    while(*ptr){
      switch(*ptr){
        case '\\':
          ptr++;
          switch(*ptr){
            case 'n':
              *out++='\n';
              break;
            case 'r':
              *out++='\r';
              break;
            case 'b':
              *out++='\b';
              break;
            case 'v':
              *out++='\v';
              break;
            case 'a':
              *out++='\a';
              break;
            case 'f':
              *out++='\f';
              break;
            case 't':
              *out++='\t';
              break;
            case '\\':
              *out++='\\';
              break;
            case '"':
              *out++='"';
              break;
            case 'x':
              ptr++;
              v1=*ptr++;
              if(!v1) return FALSE;
              v2=*ptr;
              if(!v2) return FALSE;
              h=v1<='9'?v1-'0':toupper(v1)-'A'+10;
              l=v2<='9'?v2-'0':toupper(v2)-'A'+10;
              *out++=(h<<4)+l;
              break;
            default:
              *out++=*ptr;
              break;
            }
          break;
        case '"':
          *out='\0';
          return TRUE;
        default:
          *out++=*ptr;
          break;
        }
      ptr++;
      }
    *value='\0';
    return FALSE;
    }

  // Non-quoted string copy sequence of non-white space
  else{
    while(*ptr && !isspace((FXuchar)*ptr) && isprint((FXuchar)*ptr)){
      *out++=*ptr++;
      }
    *out='\0';
    }
  return TRUE;
  }



// Unparse registry file
FXbool FXSettings::unparseFile(const FXString& filename){
  FXchar buffer[MAXVALUE];
  FXStringDict *group;
  FILE *file;
  FXint s,e;
  FXbool sec,mrk;
  file=fopen(filename.text(),"w");
  if(file){
    FXTRACE((100,"Writing settings file: %s\n",filename.text()));

    // Loop over all sections
    for(s=first(); s<size(); s=next(s)){

      // Get group
      group=data(s);
      FXASSERT(group);
      sec=FALSE;

      // Loop over all entries
      for(e=group->first(); e<group->size(); e=group->next(e)){
        mrk=group->mark(e);

        // Write section name if not written yet
        if(mrk && !sec){
          FXASSERT(key(s));
          fputc('[',file);
          fputs(key(s),file);
          fputc(']',file);
          fputc('\n',file);
          sec=TRUE;
          }

        // Only write marked entries
        if(mrk){
          FXASSERT(group->key(e));
          FXASSERT(group->data(e));

          // Write key name
          fputs(group->key(e),file);
          fputc('=',file);

          // Write quoted value
          if(unparseValue(buffer,group->data(e))){
            fputc('"',file);
            fputs(buffer,file);
            fputc('"',file);
            }

          // Write unquoted
          else{
            fputs(buffer,file);
            }

          // End of line
          fputc('\n',file);
          }
        }

      // Blank line after end
      if(sec){
        fputc('\n',file);
        }
      }
    fclose(file);
    return TRUE;
    }
  return FALSE;
  }


// Unparse value by quoting strings; return TRUE if quote needed
FXbool FXSettings::unparseValue(FXchar* buffer,const FXchar* value){
  const FXchar hex[]="0123456789ABCDEF";
  register FXchar *ptr=buffer;
  register FXbool mustquote=FALSE;
  register FXuint v;
  FXASSERT(value);
  while(*value && ptr<&buffer[MAXVALUE-5]){
    switch(*value){
      case '\n':
        *ptr++='\\';
        *ptr++='n';
        mustquote=TRUE;
        break;
      case '\r':
        *ptr++='\\';
        *ptr++='r';
        mustquote=TRUE;
        break;
      case '\b':
        *ptr++='\\';
        *ptr++='b';
        mustquote=TRUE;
        break;
      case '\v':
        *ptr++='\\';
        *ptr++='v';
        mustquote=TRUE;
        break;
      case '\a':
        *ptr++='\\';
        *ptr++='a';
        mustquote=TRUE;
        break;
      case '\f':
        *ptr++='\\';
        *ptr++='f';
        mustquote=TRUE;
        break;
      case '\t':
        *ptr++='\\';
        *ptr++='t';
        mustquote=TRUE;
        break;
      case '\\':
        *ptr++='\\';
        *ptr++='\\';
        mustquote=TRUE;
        break;
      case '"':
        *ptr++='\\';
        *ptr++='"';
        mustquote=TRUE;
        break;
      case ' ':
        *ptr++=' ';
        mustquote=TRUE;
        break;
      default:
        v=*value;
        if(v<0x20 || 0x7f<v){
          *ptr++='\\';
          *ptr++='x';
          *ptr++=hex[((v>>4)&15)];
          *ptr++=hex[v&15];
          mustquote=TRUE;
          }
        else{
          *ptr++=v;
          }
        break;
      }
    value++;
    }
  FXASSERT(ptr<&buffer[MAXVALUE]);
  *ptr='\0';
  return mustquote;
  }


// Furnish our own version if we have to
#ifndef HAVE_VSSCANF
extern "C" int vsscanf(const char* str, const char* format, va_list arg_ptr);
#endif


// Read a formatted registry entry
FXint FXSettings::readFormatEntry(const FXchar *section,const FXchar *key,const FXchar *fmt,...){
  if(!section){ fxerror("FXSettings::readFormatEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::readFormatEntry: NULL key argument.\n"); }
  if(!fmt){ fxerror("FXSettings::readFormatEntry: NULL fmt argument.\n"); }
  FXStringDict *group=find(section);
  va_list args;
  va_start(args,fmt);
  FXint result=0;
  if(group){
    const char *value=group->find(key);
    if(value){
      result=vsscanf((char*)value,fmt,args);    // Cast needed for HP-UX 11, which has wrong prototype for vsscanf
      }
    }
  va_end(args);
  return result;
  }


// Read a string-valued registry entry
const FXchar *FXSettings::readStringEntry(const FXchar *section,const FXchar *key,const FXchar *def){
  if(!section){ fxerror("FXSettings::readStringEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::readStringEntry: NULL key argument.\n"); }
  FXStringDict *group=find(section);
  if(group){
    const char *value=group->find(key);
    if(value) return value;
    }
  return def;
  }


// Read a int-valued registry entry
FXint FXSettings::readIntEntry(const FXchar *section,const FXchar *key,FXint def){
  if(!section){ fxerror("FXSettings::readIntEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::readIntEntry: NULL key argument.\n"); }
  FXStringDict *group=find(section);
  if(group){
    const char *value=group->find(key);
    if(value){
      FXint ivalue;
      if(value[0]=='0' && (value[1]=='x' || value[1]=='X')){
        if(sscanf(value+2,"%x",&ivalue)) return ivalue;
        }
      else{
        if(sscanf(value,"%d",&ivalue)==1) return ivalue;
        }
      }
    }
  return def;
  }


// Read a unsigned int-valued registry entry
FXuint FXSettings::readUnsignedEntry(const FXchar *section,const FXchar *key,FXuint def){
  if(!section){ fxerror("FXSettings::readUnsignedEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::readUnsignedEntry: NULL key argument.\n"); }
  FXStringDict *group=find(section);
  if(group){
    const char *value=group->find(key);
    if(value){
      FXuint ivalue;
      if(value[0]=='0' && (value[1]=='x' || value[1]=='X')){
        if(sscanf(value+2,"%x",&ivalue)) return ivalue;
        }
      else{
        if(sscanf(value,"%u",&ivalue)==1) return ivalue;
        }
      }
    }
  return def;
  }


// Read a double-valued registry entry
FXdouble FXSettings::readRealEntry(const FXchar *section,const FXchar *key,FXdouble def){
  if(!section){ fxerror("FXSettings::readRealEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::readRealEntry: NULL key argument.\n"); }
  FXStringDict *group=find(section);
  if(group){
    const char *value=group->find(key);
    if(value){
      FXdouble dvalue;
      if(sscanf(value,"%lf",&dvalue)==1) return dvalue;
      }
    }
  return def;
  }


// Read a color registry entry
FXColor FXSettings::readColorEntry(const FXchar *section,const FXchar *key,FXColor def){
  if(!section){ fxerror("FXSettings::readColorEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::readColorEntry: NULL key argument.\n"); }
  FXStringDict *group=find(section);
  if(group){
    const char *value=group->find(key);
    if(value){
      return fxcolorfromname(value);
      }
    }
  return def;
  }


// Write a formatted registry entry
FXint FXSettings::writeFormatEntry(const FXchar *section,const FXchar *key,const FXchar *fmt,...){
  if(!section){ fxerror("FXSettings::writeFormatEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::writeFormatEntry: NULL key argument.\n"); }
  if(!fmt){ fxerror("FXSettings::writeFormatEntry: NULL fmt argument.\n"); }
  FXStringDict *group=insert(section);
  va_list args;
  va_start(args,fmt);
  FXint result=0;
  if(group){
    FXchar buffer[2000];
#if defined(__GLIBC__) || defined(WIN32)                // Try to be safe about it...
    result=vsnprintf(buffer,sizeof(buffer),fmt,args);
#else
    result=vsprintf(buffer,fmt,args);
#endif
    group->replace(key,buffer,TRUE);
    modified=TRUE;
    }
  va_end(args);
  return result;
  }


// Write a string-valued registry entry
FXbool FXSettings::writeStringEntry(const FXchar *section,const FXchar *key,const FXchar *val){
  if(!section){ fxerror("FXSettings::writeStringEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::writeStringEntry: NULL key argument.\n"); }
  FXStringDict *group=insert(section);
  if(group){
    group->replace(key,val,TRUE);
    modified=TRUE;
    return TRUE;
    }
  return FALSE;
  }


// Write a int-valued registry entry
FXbool FXSettings::writeIntEntry(const FXchar *section,const FXchar *key,FXint val){
  if(!section){ fxerror("FXSettings::writeIntEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::writeIntEntry: NULL key argument.\n"); }
  FXStringDict *group=insert(section);
  if(group){
    FXchar buffer[10];
    sprintf(buffer,"%d",val);
    group->replace(key,buffer,TRUE);
    modified=TRUE;
    return TRUE;
    }
  return FALSE;
  }


// Write a unsigned int-valued registry entry
FXbool FXSettings::writeUnsignedEntry(const FXchar *section,const FXchar *key,FXuint val){
  if(!section){ fxerror("FXSettings::writeUnsignedEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::writeUnsignedEntry: NULL key argument.\n"); }
  FXStringDict *group=insert(section);
  if(group){
    FXchar buffer[10];
    sprintf(buffer,"%u",val);
    group->replace(key,buffer,TRUE);
    modified=TRUE;
    return TRUE;
    }
  return FALSE;
  }


// Write a double-valued registry entry
FXbool FXSettings::writeRealEntry(const FXchar *section,const FXchar *key,FXdouble val){
  if(!section){ fxerror("FXSettings::writeRealEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::writeRealEntry: NULL key argument.\n"); }
  FXStringDict *group=insert(section);
  if(group){
    FXchar buffer[60];
    sprintf(buffer,"%.16g",val);
    group->replace(key,buffer,TRUE);
    modified=TRUE;
    return TRUE;
    }
  return FALSE;
  }


// Write a color registry entry
FXbool FXSettings::writeColorEntry(const FXchar *section,const FXchar *key,FXColor val){
  if(!section){ fxerror("FXSettings::writeColorEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::writeColorEntry: NULL key argument.\n"); }
  FXStringDict *group=insert(section);
  if(group){
    FXchar buffer[60];
    group->replace(key,fxnamefromcolor(buffer,val),TRUE);
    modified=TRUE;
    return TRUE;
    }
  return FALSE;
  }


// Delete a registry entry
FXbool FXSettings::deleteEntry(const FXchar *section,const FXchar *key){
  if(!section){ fxerror("FXSettings::deleteEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::deleteEntry: NULL key argument.\n"); }
  FXStringDict *group=insert(section);
  if(group){
    group->remove(key);
    modified=TRUE;
    return TRUE;
    }
  return FALSE;
  }


// Delete section
FXbool FXSettings::deleteSection(const FXchar *section){
  if(!section){ fxerror("FXSettings::deleteSection: NULL section argument.\n"); }
  remove(section);
  modified=TRUE;
  return TRUE;
  }


// Clear all sections
FXbool FXSettings::clear(){
  FXDict::clear();
  modified=TRUE;
  return TRUE;
  }


// See if section exists
FXbool FXSettings::existingSection(const FXchar *section){
  if(!section){ fxerror("FXSettings::existingSection: NULL section argument.\n"); }
  return find(section)!=NULL;
  }


// See if entry exists
FXbool FXSettings::existingEntry(const FXchar *section,const FXchar *key){
  if(!section){ fxerror("FXSettings::existingEntry: NULL section argument.\n"); }
  if(!key){ fxerror("FXSettings::existingEntry: NULL key argument.\n"); }
  FXStringDict *group=find(section);
  return group && group->find(key)!=NULL;
  }


// Clean up
FXSettings::~FXSettings(){
  clear();
  }
