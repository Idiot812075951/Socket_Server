@echo off
set year=%date:~0,4%
set month=%date:~5,2%
set day=%date:~8,2%
set filename=bspacedb
set username=bspace
set password=y13zowee
set listener=orcl
set directory=D:\oracledb\dbback
exp %username%/%password%@%listener% grants=N file=%directory%/%filename%.dmp INDEXES=n STATISTICS=none  log='D:\oracledb\dbremark\export.log'