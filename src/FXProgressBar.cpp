/********************************************************************************
*                                                                               *
*                      P r o g r e s s B a r   W i d g e t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* Contributed by: Jonathan Bush and Sander Jansen <sxj@cfdrc.com>               *
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
* $Id: FXProgressBar.cpp,v 1.26.4.3 2003/06/20 19:02:07 fox Exp $                *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXProgressBar.h"


/*
  Notes:
  - Reduced flicker by not drawing background at all.
  - Reduced flicker by setting clip rectangle to only redraw interior.
  - Progress bar has a target, as it can send update messages.
*/

#define PROGRESSBAR_MASK (PROGRESSBAR_PERCENTAGE|PROGRESSBAR_VERTICAL|PROGRESSBAR_DIAL)


/*******************************************************************************/


// Map
FXDEFMAP(FXProgressBar) FXProgressBarMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXProgressBar::onPaint),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXProgressBar::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXProgressBar::onCmdSetIntValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXProgressBar::onCmdGetIntValue),
  };


// Object implementation
FXIMPLEMENT(FXProgressBar,FXFrame,FXProgressBarMap,ARRAYNUMBER(FXProgressBarMap))



// Make progress bar
FXProgressBar::FXProgressBar(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXFrame(p,opts,x,y,w,h,pl,pr,pt,pb){
  target=tgt;
  message=sel;
  progress=0;
  total=100;
  if(opts&PROGRESSBAR_DIAL){
    textNumColor=FXRGB(255,255,255);
    barBGColor=FXRGB(0,0,0);
    barBGColor=getApp()->getBackColor();
    textNumColor=FXRGB(0,0,255);
    barsize=60;
    }
  else{
    barBGColor=getApp()->getBackColor();
    textNumColor=FXRGB(0,0,255);
    barsize=5;
    }
  font=getApp()->getNormalFont();
  barColor=FXRGB(0,0,255);
  textAltColor=FXRGB(255,255,255);
  backColor=barBGColor;
  }


// Get minimum width
FXint FXProgressBar::getDefaultWidth(){
  FXint w=1,t;
  if((options&PROGRESSBAR_VERTICAL) || (options&PROGRESSBAR_DIAL)){
    w=barsize;
    if(options&PROGRESSBAR_PERCENTAGE){
      t=font->getTextWidth("100%",4);
      if(w<t) w=t;
      }
    }
  return w+padleft+padright+(border<<1);
  }


// Get minimum height
FXint FXProgressBar::getDefaultHeight(){
  FXint h=1,t;
  if(!(options&PROGRESSBAR_VERTICAL) || (options&PROGRESSBAR_DIAL)){
    h=barsize;
    if(options&PROGRESSBAR_PERCENTAGE){
      t=font->getFontHeight();
      if(h<t) h=t;
      }
    }
  return h+padtop+padbottom+(border<<1);
  }


// Create window
void FXProgressBar::create(){
  FXFrame::create();
  font->create();
  }


// Detach window
void FXProgressBar::detach(){
  FXFrame::detach();
  font->detach();
  }


// Update progress value from a message
long FXProgressBar::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setProgress((FXuint)(FXuval)ptr);
  return 1;
  }


// Set value
long FXProgressBar::onCmdSetIntValue(FXObject*,FXSelector,void* ptr){
  setProgress(*((FXint*)ptr));
  return 1;
  }


// Get value
long FXProgressBar::onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
  *((FXint*)ptr)=getProgress();
  return 1;
  }


// Draw the progress bar
long FXProgressBar::onPaint(FXObject*,FXSelector,void *ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXDCWindow dc(this,event);
  FXint percent,barlength,barfilled,tx,ty,tw,th,n,d;
  FXchar numtext[5];

  // Draw borders if any
  drawFrame(dc,0,0,width,height);

  if(options&PROGRESSBAR_DIAL){

    // If total is 0, it's 100%
    barfilled=23040;
    percent=100;
    if(total!=0){
      barfilled=(FXuint) (((double)progress * (double)23040) / (double)total);
      percent=(FXuint) (((double)progress * 100.0) / (double)total);
      }

    tw=width-(border<<1)-padleft-padright;
    th=height-(border<<1)-padtop-padbottom;
    d=FXMIN(tw,th)-1;

    tx=border+padleft+((tw-d)/2);
    ty=border+padtop+((th-d)/2);

    dc.setForeground(getBaseColor());
    dc.fillRectangle(border,border,width-(border<<1),height-(border<<1));

    if(barfilled!=23040){
      dc.setForeground(barBGColor);
      dc.fillArc(tx,ty,d,d,5760,23040-barfilled);
      }
    if(barfilled!=0){
      dc.setForeground(barColor);
      dc.fillArc(tx,ty,d,d,5760,-barfilled);
      }

    dc.setForeground(borderColor);
    dc.drawArc(tx+1,ty,d,d,90*64,45*64);
    dc.drawArc(tx,ty+1,d,d,135*64,45*64);
    dc.setForeground(baseColor);
    dc.drawArc(tx-1,ty,d,d,270*64,45*64);
    dc.drawArc(tx,ty-1,d,d,315*64,45*64);

    dc.setForeground(shadowColor);
    dc.drawArc(tx,ty,d,d,45*64,180*64);
    dc.setForeground(hiliteColor);
    dc.drawArc(tx,ty,d,d,225*64,180*64);

    // Draw text
    if(options&PROGRESSBAR_PERCENTAGE){
      dc.setTextFont(font);
      tw=font->getTextWidth("100%",4);
      if(tw>(11*d)/16) return 1;
      th=font->getFontHeight();
      if(th>d/2) return 1;
      sprintf(numtext,"%d%%",percent);
      n=strlen(numtext);
      tw=font->getTextWidth(numtext,n-1);       // Not including the % looks better
      th=font->getFontHeight();
      tx=tx+d/2-tw/2;
      ty=ty+d/2+font->getFontAscent()+5;
      //dc.setForeground(textNumColor);
      dc.setForeground(FXRGB(255,255,255));
      dc.setFunction(BLT_SRC_XOR_DST);      // FIXME
      dc.drawText(tx,ty,numtext,n);
      }
    }

  // Vertical bar
  else if(options&PROGRESSBAR_VERTICAL){

    // If total is 0, it's 100%
    barlength=height-border-border;
    barfilled=barlength;
    percent=100;
    if(total!=0){
      barfilled=(FXuint) (((double)progress * (double)barlength) / (double)total);
      percent=(FXuint) (((double)progress * 100.0) / (double)total);
      }

    // Draw completed bar
    if(0<barfilled){
      dc.setForeground(barColor);
      dc.fillRectangle(border,height-border-barfilled,width-(border<<1),barfilled);
      }

    // Draw uncompleted bar
    if(barfilled<barlength){
      dc.setForeground(barBGColor);
      dc.fillRectangle(border,border,width-(border<<1),barlength-barfilled);
      }

    // Draw text
    if(options&PROGRESSBAR_PERCENTAGE){
      dc.setTextFont(font);
      sprintf(numtext,"%d%%",percent);
      n=strlen(numtext);
      tw=font->getTextWidth(numtext,n);
      th=font->getFontHeight();
      ty=(height-th)/2+font->getFontAscent();
      tx=(width-tw)/2;
      if(height-border-barfilled>ty){           // In upper side
        dc.setForeground(textNumColor);
        dc.drawText(tx,ty,numtext,n);
        }
      else if(ty-th>height-border-barfilled){   // In lower side
        dc.setForeground(textAltColor);
        dc.drawText(tx,ty,numtext,n);
        }
      else{                                     // In between!
        dc.setForeground(textAltColor);
        dc.setClipRectangle(border,height-border-barfilled,width-(border<<1),barfilled);
        dc.drawText(tx,ty,numtext,n);
        dc.setForeground(textNumColor);
        dc.setClipRectangle(border,border,width-(border<<1),barlength-barfilled);
        dc.drawText(tx,ty,numtext,n);
        dc.clearClipRectangle();
        }
      }
    }

  // Horizontal bar
  else{

    // If total is 0, it's 100%
    barlength=width-border-border;
    barfilled=barlength;
    percent=100;
    if(total!=0){
      barfilled=(FXuint) (((double)progress * (double)barlength) / (double)total);
      percent=(FXuint) (((double)progress * 100.0) / (double)total);
      }

    // Draw completed bar
    if(0<barfilled){
      dc.setForeground(barColor);
      dc.fillRectangle(border,border,barfilled,height-(border<<1));
      }

    // Draw uncompleted bar
    if(barfilled<barlength){
      dc.setForeground(barBGColor);
      dc.fillRectangle(border+barfilled,border,barlength-barfilled,height-(border<<1));
      }

    // Draw text
    if(options&PROGRESSBAR_PERCENTAGE){
      dc.setTextFont(font);
      sprintf(numtext,"%d%%",percent);
      n=strlen(numtext);
      tw=font->getTextWidth(numtext,n-1); // Not including the % looks better
      th=font->getFontHeight();
      ty=(height-th)/2+font->getFontAscent();
      tx=(width-tw)/2;
      if(border+barfilled<=tx){           // In right side
        dc.setForeground(textNumColor);
        dc.drawText(tx,ty,numtext,n);
        }
      else if(tx+tw<=border+barfilled){   // In left side
        dc.setForeground(textAltColor);
        dc.drawText(tx,ty,numtext,n);
        }
      else{                               // In between!
        dc.setForeground(textAltColor);
        dc.setClipRectangle(border,border,barfilled,height);
        dc.drawText(tx,ty,numtext,n);
        dc.setForeground(textNumColor);
        dc.setClipRectangle(border+barfilled,border,barlength-barfilled,height);
        dc.drawText(tx,ty,numtext,n);
        dc.clearClipRectangle();
        }
      }
    }
  return 1;
  }


// Set amount of progress made
void FXProgressBar::setProgress(FXuint value){
  if(value>total) value=total;
  if(value!=progress){
    progress=value;
    update(border,border,width-(border<<1),height-(border<<1));
    repaint(border,border,width-(border<<1),height-(border<<1));
    getApp()->flush();
    }
  }


// Increment amount of progress
void FXProgressBar::increment(FXuint value){
  if(value!=0){
    progress+=value;
    if(progress>total) progress=total;
    update(border,border,width-(border<<1),height-(border<<1));
    repaint(border,border,width-(border<<1),height-(border<<1));
    getApp()->flush();
    }
  }


// Set total amount to completion
void FXProgressBar::setTotal(FXuint value){
  if(value!=total){
    total=value;
    update(border,border,width-(border<<1),height-(border<<1));
    repaint(border,border,width-(border<<1),height-(border<<1));
    getApp()->flush();
    }
  }


// Change bar color
void FXProgressBar::setBarColor(FXColor clr){
  if(barColor!=clr){
    barColor=clr;
    update(border,border,width-(border<<1),height-(border<<1));
    }
  }


// Change bar background color
void FXProgressBar::setBarBGColor(FXColor clr){
  if(barBGColor!=clr){
    barBGColor=clr;
    update(border,border,width-(border<<1),height-(border<<1));
    }
  }


// Change text foreground color
void FXProgressBar::setTextColor(FXColor clr){
  if(textNumColor!=clr){
    textNumColor=clr;
    update();
    }
  }


// Change alternate text color
void FXProgressBar::setTextAltColor(FXColor clr) {
  if(textAltColor!=clr){
    textAltColor=clr;
    update();
    }
  }


// Hide percentage display
void FXProgressBar::hideNumber(){
  if(options&PROGRESSBAR_PERCENTAGE){
    options&=~PROGRESSBAR_PERCENTAGE;
    recalc();
    update();
    }
  }


// Show percentage display
void FXProgressBar::showNumber(){
  if(!(options&PROGRESSBAR_PERCENTAGE)){
    options|=PROGRESSBAR_PERCENTAGE;
    recalc();
    update();
    }
  }


void FXProgressBar::setBarSize(FXint size){
  if(size<1){ fxerror("%s::setBarSize: zero or negative barsize specified.\n",getClassName()); }
  if(barsize!=size){
    barsize=size;
    recalc();
    update();
    }
  }


// Change the font
void FXProgressBar::setFont(FXFont *fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update();
    }
  }


// Change spinner style
void FXProgressBar::setBarStyle(FXuint style){
  FXuint opts=(options&~PROGRESSBAR_MASK) | (style&PROGRESSBAR_MASK);
  if(options!=opts){
    options=opts;
    recalc();
    }
  }


// Get spinner style
FXuint FXProgressBar::getBarStyle() const {
  return (options&PROGRESSBAR_MASK);
  }


// Save object to stream
void FXProgressBar::save(FXStream& store) const {
  FXFrame::save(store);
  store << progress;
  store << total;
  store << barsize;
  store << font;
  store << barBGColor;
  store << barColor;
  store << textNumColor;
  store << textAltColor;
  }


// Load object from stream
void FXProgressBar::load(FXStream& store){
  FXFrame::load(store);
  store >> progress;
  store >> total;
  store >> barsize;
  store >> font;
  store >> barBGColor;
  store >> barColor;
  store >> textNumColor;
  store >> textAltColor;
  }



// Destroy
FXProgressBar::~FXProgressBar(){
  font=(FXFont*)-1;
  }



