/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * ob_sql_pool.h is for what ...
 *
 * Version: ***: ob_sql_conn_acquire.h  Thu Nov  8 16:02:14 2012 fangji.hcm Exp $
 *
 * Authors:
 *   Author fangji
 *   Email: fangji.hcm@alipay.com
 *     -some work detail if you want
 *
 */
#ifndef OB_SQL_CONN_ACQUIRE_H_
#define OB_SQL_CONN_ACQUIRE_H_

#include "ob_sql_define.h"


OB_SQL_CPP_START
#include <pthread.h>
#include <stdlib.h>
#include "ob_sql_list.h"
#include "ob_sql_init.h"
#include <pthread.h>
#include "ob_sql_struct.h"
#include "ob_sql_select_method.h"
#include "ob_sql_data_source.h"
#include "ob_sql_group_data_source.h"



/**
 * 根据传入的算法从指定集群中找到相应的连接
 *
 * @param cluster      集群信息
 * @param sql          sql语句
 * @param length       sql语句的长度
 */
ObSQLConn* acquire_conn(ObClusterInfo* cluster,
                        const char* sql, unsigned long length);

/**
 * 根据传入的算法从指定集群中找到相应的连接
 *
 * @param gds          global group data source
 * @param method       method of selection
 * @param cluster_id   集群的id号
 * @param sql          sql语句
 * @param length       sql语句的长度
 */
ObSQLConn* acquire_conn_random(ObGroupDataSource *pool);

/**
 * 关闭当前连接，新建一条到同一个DS的连接
 * 在发现连接上有错误时调用
 * @param conn   出错的连接
 *
 */
int reconnect(ObSQLConn *conn);

/* 用户把连接还给连接池 从busy上删除 连到free list上 */
int release_conn(ObSQLConn *conn);

/**
 * 移动DS上的连接到一个ObSQLConnList上
 * ms下线时需要把对应DS上的连接信息移动到一个ConnList上
 * 再链接到g_delete_ms_list上，等待时机删除
 * @param dest     目的conn list
 * @param src      源DataSource
 */
//int move_conn_list(ObSQLConnList *dest, ObDataSource *src);

/**
 * 在不同的ClusterInfo之间移动连接信息
 * @param dinfo   dest cluster info
 * @param sinfo   src  cluster info
 */
//int move_conn(ObClusterInfo *dinfo, ObClusterInfo *sinfo);
OB_SQL_CPP_END

#endif
