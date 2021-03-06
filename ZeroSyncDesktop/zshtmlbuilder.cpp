/* =========================================================================
   ZSHtmlBuilder - Class that creates the ZeroWebIndex html file


   -------------------------------------------------------------------------
   Copyright (c) 2014 Bernhard Finger
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

#include "zshtmlbuilder.h"

ZShtmlBuilder::ZShtmlBuilder(QObject *parent) :
    QObject(parent)
{

}


/**
  * @brief ZShtmlBuilder::slotGenerateHtml reloads the ZeroWebIndex.html file, for the web presentation
  *
**/
void ZShtmlBuilder::slotGenerateHtml()
{
    QFile file("../ZeroWebIndex.html");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "File opening failed. \n";
        return;
    }

    QString tmp = formHtml();

    QTextStream out(&file);
    out << tmp << "\n";
}

/**
 * @brief ZShtmlBuilder::formHtml forms the data from the database, as an html String, to prepare to embed in a .html file.
 * @return the full String with all paths.
 */
QString ZShtmlBuilder::formHtml()
{
    QSqlQuery query = ZSDatabase::getInstance()->fetchAllUndeletedEntries();

    ZSTree *fileTree = new ZSTree();

    // HTML Header for the .html file, where this function is used for.
    QString result = "<!DOCTYPE html>\n<html>\n<head><meta charset=\"UTF-8\" /><title>Zero WEB Index</title>\n";
    result += "<link rel=\"stylesheet\" type=\"text/css\" href=\"dist/css/bootstrap.min.css\" />\n";
    result += "<link rel=\"stylesheet\" type=\"text/css\" href=\"design/CSS/menu_template.css\" />\n";
    result += "</head>\n<body style=\"background-color: #EDEDED;\" onload=\"showRoot()\">";

    result += "\n<div class=\"row\">\n <div class=\"col-lg-2\" id=\"leftSpace\"></div> \n";
    result += "<div class=\"col-lg-8\" id=\"midSpace\">\n <div class=\"row\">\n <div class=\"col-lg-3\">";
    result += "<div class=\"logo\"><img src=\"ZS_logo.png\" id=\"logo\" /></div></div>\n";
    result += "<div class=\"col-lg-6\"><div class=\"header\"><h1>Zero Sync Web</h1></div></div>";
    result += "<div class=\"col-lg-3\">\n<div class=\"upload pull-right\">\n";
    result += "<div style=\"display:none;\"><input type=\"file\" id=\"file_input\" onchange=\"readUploadedFile();\" /></div>\n";
    result += "<button type=\"button\" class=\"btn btn-default btn-lg\" onclick=\"$('#file_input').click();\">\n<span class=\"glyphicon glyphicon-cloud-upload\"></span> Upload \n";
    result += "</button></div></div></div>\n";
    result += "<div class=\"nav\">\n<p><ol class=\"breadcrumb\">\n<li><a href=\"javascript:showRoot()\">Index</a></ol></p></div>\n";
    result += "<div class=\"panel panel-default\"><h3>Synchronized files</h3>\n<div class=\"panel heading\"></div>\n";
    result += "<table class=\"table table-hover\">\n";
    result += "<tr id=\"goBack\" style=\"display: none;\"></tr>";

    while(query.next())
    {   
        QString data = query.value(0).toString();

        fileTree->append(data);
    }
    result += fileTree->toHtmlString();
    result += "</table>\n";

    result += "<script src=\"dist/js/jquery-2.1.0.min.js\"></script>\n";
    result += "<script src=\"dist/js/bootstrap.min.js\"></script>\n";
    result += "<script src=\"functionality/functions.js\"></script>\n";
    result += "</body>\n</html>";

    return result;
}
