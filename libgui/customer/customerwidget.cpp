/*
 * customerwidget.cpp
 * Copyright 2017 - ~, Apin <apin.klas@gmail.com>
 *
 * This file is part of Turbin.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "customerwidget.h"
#include "ui_normalwidget.h"
#include "horizontalheader.h"
#include "tablewidget.h"
#include "tablemodel.h"
#include "tableitem.h"
#include "global_constant.h"
#include "message.h"
#include "tableview.h"
#include "usersession.h"
#include "guiutil.h"
#include "headerwidget.h"
#include "db_constant.h"
#include "flashmessagemanager.h"
#include "customeradddialog.h"
#include <QMessageBox>

using namespace LibGUI;
using namespace LibG;

CustomerWidget::CustomerWidget(LibG::MessageBus *bus, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NormalWidget),
    mTableWidget(new TableWidget(this)),
    mAddDialog(new CustomerAddDialog(bus, this))
{
    setMessageBus(bus);
    ui->setupUi(this);
    ui->labelTitle->setText(tr("User"));
    ui->verticalLayout->addWidget(mTableWidget);
    mTableWidget->initCrudButton();
    auto model = mTableWidget->getModel();
    model->setMessageBus(bus);
    model->addColumn("number", tr("Number"));
    model->addColumn("name", tr("Name"));
    model->addColumn("phone", tr("Phone"));
    model->addColumn("address", tr("Address"));
    model->addColumnMoney("point", tr("Point"));
    model->addColumnMoney("debt", tr("Debt"));
    model->addHeaderFilter("number", HeaderFilter{HeaderWidget::LineEdit, TableModel::FilterLike, QVariant()});
    model->addHeaderFilter("name", HeaderFilter{HeaderWidget::LineEdit, TableModel::FilterLike, QVariant()});
    model->setTypeCommand(MSG_TYPE::CUSTOMER, MSG_COMMAND::QUERY);
    mTableWidget->setupTable();
    GuiUtil::setColumnWidth(mTableWidget->getTableView(), QList<int>() << 150 << 150 << 150 << 150);
    mTableWidget->getTableView()->horizontalHeader()->setStretchLastSection(true);
    model->refresh();
    connect(mTableWidget, SIGNAL(addClicked()), SLOT(addClicked()));
    connect(mTableWidget, SIGNAL(updateClicked(QModelIndex)), SLOT(updateClicked(QModelIndex)));
    connect(mTableWidget, SIGNAL(deleteClicked(QModelIndex)), SLOT(deleteClicked(QModelIndex)));
    connect(mAddDialog, SIGNAL(customerAdded()), SLOT(customerAdded()));
    connect(mAddDialog, SIGNAL(customerUpdated(int)), SLOT(customerUpdated(int)));
}

void CustomerWidget::messageReceived(LibG::Message *msg)
{
    if(msg->isTypeCommand(MSG_TYPE::CUSTOMER, MSG_COMMAND::DEL)) {
        if(msg->isSuccess()) {
            FlashMessageManager::showMessage(tr("Customer deleted successfully"));
            mTableWidget->getModel()->refresh();
        }
    }
}

void CustomerWidget::addClicked()
{
    mAddDialog->reset();
    mAddDialog->show();
}

void CustomerWidget::updateClicked(const QModelIndex &index)
{
    if(!index.isValid()) return;
    auto item = static_cast<TableItem*>(index.internalPointer());
    mAddDialog->fill(item->data());
    mAddDialog->show();
}

void CustomerWidget::deleteClicked(const QModelIndex &index)
{
    if(!index.isValid()) return;
    auto item = static_cast<TableItem*>(index.internalPointer());
    int ret = QMessageBox::question(this, tr("Delete Confirmation"), tr("Are you sure to delete the customer?"));
    if(ret == QMessageBox::Yes) {
        Message msg(MSG_TYPE::CUSTOMER, MSG_COMMAND::DEL);
        msg.addData("id", item->id);
        sendMessage(&msg);
    }
}

void CustomerWidget::customerAdded()
{
    mTableWidget->getModel()->refresh();
}

void CustomerWidget::customerUpdated(int id)
{
    mTableWidget->getModel()->resfreshOne(id);
}
