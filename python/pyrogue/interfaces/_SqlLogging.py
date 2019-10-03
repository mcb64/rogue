#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : PyRogue - SQL Logging Module
#-----------------------------------------------------------------------------
# File       : pyrogue/_sqlLogging.py
# Created    : 2019-09-03
#-----------------------------------------------------------------------------
# This file is part of the rogue software platform. It is subject to 
# the license terms in the LICENSE.txt file found in the top-level directory 
# of this distribution and at: 
#    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
# No part of the rogue software platform, including this file, may be 
# copied, modified, propagated, or distributed except according to the terms 
# contained in the LICENSE.txt file.
#-----------------------------------------------------------------------------

import pyrogue as pr
import sqlalchemy

class SqlLogger(object):

    def __init__(self, url):
        self._log = pr.logInit(cls=self,name="SqlLogger",path=None)
        self._url = url
        self._conn = None

        try:
            conn = sqlalchemy.create_engine(self._url)
            self._log.info("Opened database connection to {}".format(self._url))
        except Exception as e:
            self._log.error("Failed to open database connection to {}: {}".format(self._url,e))
            return

        self._metadata = sqlalchemy.MetaData(conn)

        self._varTable = sqlalchemy.Table('variables', self._metadata,
            sqlalchemy.Column('id',        sqlalchemy.Integer, primary_key=True),
            sqlalchemy.Column('timestamp', sqlalchemy.DateTime(timezone=True), server_default=sqlalchemy.func.now()),
            sqlalchemy.Column('path',      sqlalchemy.String),
            sqlalchemy.Column('enum',      sqlalchemy.String),
            sqlalchemy.Column('disp',      sqlalchemy.String),
            sqlalchemy.Column('value',     sqlalchemy.String),
            sqlalchemy.Column('severity',  sqlalchemy.String),
            sqlalchemy.Column('status',    sqlalchemy.String))

        self._logTable = sqlalchemy.Table('syslog', self._metadata,
            sqlalchemy.Column('id',          sqlalchemy.Integer, primary_key=True),
            sqlalchemy.Column('timestamp',   sqlalchemy.DateTime(timezone=True), server_default=sqlalchemy.func.now()),
            sqlalchemy.Column('name',        sqlalchemy.String),
            sqlalchemy.Column('message',     sqlalchemy.String),
            sqlalchemy.Column('exception',   sqlalchemy.String),
            sqlalchemy.Column('levelName',   sqlalchemy.String),
            sqlalchemy.Column('levelNumber', sqlalchemy.Integer))

        self._varTable.create(conn, checkfirst=True)
        self._logTable.create(conn, checkfirst=True)
        self._conn = conn

        self._queue = queue.Queue()
        self._thread = threading.Thread(target=self._worker)

    def logVariable(self, path, varValue):
        if self._conn is not None:
            self._queue.put(varValue)

    def logSyslog(self, syslogData):
        if self._conn is not None:
            self._queue.put(syslogData)

    def stop(self):
        self._queue.put(None)
        self._thread.join()

    def _worker(self):
        while True:
            ent = self._queue.get()

            # Done
            if ent is None: return

            if self._conn is not None:
                try:

                    if isinstance(ent,pyrogue.VariableValue):
                        ins = self._varTable.insert().values(path=path,
                                                             enum=str(ent.enum),
                                                             disp=ent.disp, 
                                                             value=ent.valueDisp,
                                                             severity=ent.severity, 
                                                             status=ent.status)
                    else:
                        ins = self._logTable.insert().values(name=ent['name'], 
                                                             message=ent['message'],
                                                             exception=ent['exception'],
                                                             levelName=ent['levelName'],
                                                             levelNumber=ent['levelNumber'])

                    self._conn.execute(ins)
                except Exception as e:
                    self._conn = None
                    pr.logException(self._log,e)
                    self._log.error("Lost database connection to {}".format(self._url))


class SqlReader(object):

    def __init__(self, url):
        self._log = pr.logInit(cls=self,name="SqlReader",path=None)
        self._url = url
        self._conn = None

        try:
            conn = sqlalchemy.create_engine(self._url)
            self._log.info("Opened database connection to {}".format(self._url))
        except Exception as e:
            self._log.error("Failed to open database connection to {}: {}".format(self._url,e))
            return

        self._metadata = sqlalchemy.MetaData(conn)

        self._varTable = sqlalchemy.Table('variables', self._metadata, autoload=True)
        self._logTable = sqlalchemy.Table('syslog', self._metadata, autoload=True)
        self._conn = conn

    # Make this more usefull
    def getVariable(self):
        r = self._conn.execute(sqlalchemy.select([self._varTable]))
        print(r.fetchall())

    # Make this more usefull
    def getSyslog(self, syslogData):
        r = self._conn.execute(sqlalchemy.select([self._logTable]))
        print(r.fetchall())

