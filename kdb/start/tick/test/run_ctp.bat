rem run all rdb

c:
cd \q\start\tick
for %%f in (ticker rdb show) do start "%%f" win\%%f.bat