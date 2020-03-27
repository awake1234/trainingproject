CC=gcc      #使用的编译器
#头文件
INCLUDEFLAGS=-I ./include	\
		     -I /usr/include/mysql/	\
	         -I /usr/local/include/hiredis/	\
			 -I /usr/include/fastdfs	\
			 -I /usr/include/fastcommon
#动态库所在的目录
LIBPATH=-L /usr/lib/

#编译选项
CFLAGS=-W
#需要链接的动态库
LIBS=-lmysqlclient \
	 -lfcgi \
	 -lpthread   \
	 -lhiredis	\
	 -lm	\
	 -lfdfsclient	\
     -lfastcommon
#目录路径
COMMON_PATH=common
CFG_BIN_PATH=bin_cgi
CFG_SRC_PATH=src_cgi

#生成的目标
login=$(CFG_BIN_PATH)/login.cgi
register=$(CFG_BIN_PATH)/register.cgi
upload=$(CFG_BIN_PATH)/upload.cgi
md5=$(CFG_BIN_PATH)/md5.cgi
mydisk=$(CFG_BIN_PATH)/mydisk.cgi
dealfile=$(CFG_BIN_PATH)/dealfile.cgi
sharefile=$(CFG_BIN_PATH)/sharefile.cgi
dealsharefile=$(CFG_BIN_PATH)/dealsharefile.cgi


#最终目标
target=$(login)		\
	   $(register)	\
	   $(upload)	\
	   $(md5)		\
	   $(mydisk)	\
	   $(dealfile)	\
	   $(sharefile)	\
	   $(dealsharefile)	
ALL:$(target)

#登录
$(login):$(CFG_SRC_PATH)/login_cgi.o	\
		 $(COMMON_PATH)/make_log.o	\
		 $(COMMON_PATH)/base64.o	\
		 $(COMMON_PATH)/cfg.o	\
		 $(COMMON_PATH)/cJSON.o	\
		 $(COMMON_PATH)/deal_mysql.o   \
		 $(COMMON_PATH)/des.o	\
		 $(COMMON_PATH)/md5.o	\
		 $(COMMON_PATH)/redis_op.o	\
		 $(COMMON_PATH)/util_cgi.o
	$(CC) $^   $(LIBPATH) $(LIBS) -o $@
#注册
$(register):$(CFG_SRC_PATH)/register_cgi.o	\
		 $(COMMON_PATH)/make_log.o	\
		 $(COMMON_PATH)/base64.o	\
		 $(COMMON_PATH)/cfg.o	\
		 $(COMMON_PATH)/cJSON.o	\
		 $(COMMON_PATH)/deal_mysql.o   \
		 $(COMMON_PATH)/des.o	\
		 $(COMMON_PATH)/md5.o	\
		 $(COMMON_PATH)/redis_op.o	\
		 $(COMMON_PATH)/util_cgi.o
	$(CC) $^  $(LIBPATH) $(LIBS) -o $@
#上传
$(upload):$(CFG_SRC_PATH)/upload_cgi.o	\
		 $(COMMON_PATH)/make_log.o	\
		 $(COMMON_PATH)/base64.o	\
		 $(COMMON_PATH)/cfg.o	\
		 $(COMMON_PATH)/cJSON.o	\
		 $(COMMON_PATH)/deal_mysql.o   \
		 $(COMMON_PATH)/des.o	\
		 $(COMMON_PATH)/md5.o	\
		 $(COMMON_PATH)/redis_op.o	\
		 $(COMMON_PATH)/util_cgi.o
	$(CC) $^  $(LIBPATH) $(LIBS) -o $@
#秒传
$(md5):$(CFG_SRC_PATH)/md5_cgi.o	\
		 $(COMMON_PATH)/make_log.o	\
		 $(COMMON_PATH)/base64.o	\
		 $(COMMON_PATH)/cfg.o	\
		 $(COMMON_PATH)/cJSON.o	\
		 $(COMMON_PATH)/deal_mysql.o   \
		 $(COMMON_PATH)/des.o	\
		 $(COMMON_PATH)/md5.o	\
		 $(COMMON_PATH)/redis_op.o	\
		 $(COMMON_PATH)/util_cgi.o
	$(CC) $^  $(LIBPATH) $(LIBS) -o $@
#我的文件
$(mydisk):$(CFG_SRC_PATH)/mydisk_cgi.o	\
		 $(COMMON_PATH)/make_log.o	\
		 $(COMMON_PATH)/base64.o	\
		 $(COMMON_PATH)/cfg.o	\
		 $(COMMON_PATH)/cJSON.o	\
		 $(COMMON_PATH)/deal_mysql.o   \
		 $(COMMON_PATH)/des.o	\
		 $(COMMON_PATH)/md5.o	\
		 $(COMMON_PATH)/redis_op.o	\
		 $(COMMON_PATH)/util_cgi.o
	$(CC) $^   $(LIBPATH) $(LIBS) -o $@
#处理文件
$(dealfile):$(CFG_SRC_PATH)/dealfile_cgi.o	\
		 $(COMMON_PATH)/make_log.o	\
		 $(COMMON_PATH)/base64.o	\
		 $(COMMON_PATH)/cfg.o	\
		 $(COMMON_PATH)/cJSON.o	\
		 $(COMMON_PATH)/deal_mysql.o   \
		 $(COMMON_PATH)/des.o	\
		 $(COMMON_PATH)/md5.o	\
		 $(COMMON_PATH)/redis_op.o	\
		 $(COMMON_PATH)/util_cgi.o
	$(CC) $^   $(LIBPATH) $(LIBS) -o $@
#共享列表
$(sharefile):$(CFG_SRC_PATH)/sharefile_cgi.o	\
		 $(COMMON_PATH)/make_log.o	\
		 $(COMMON_PATH)/base64.o	\
		 $(COMMON_PATH)/cfg.o	\
		 $(COMMON_PATH)/cJSON.o	\
		 $(COMMON_PATH)/deal_mysql.o   \
		 $(COMMON_PATH)/des.o	\
		 $(COMMON_PATH)/md5.o	\
		 $(COMMON_PATH)/redis_op.o	\
		 $(COMMON_PATH)/util_cgi.o
	$(CC) $^  $(LIBPATH) $(LIBS) -o $@
#处理共享文件
$(dealsharefile):$(CFG_SRC_PATH)/dealsharefile_cgi.o	\
		 $(COMMON_PATH)/make_log.o	\
		 $(COMMON_PATH)/base64.o	\
		 $(COMMON_PATH)/cfg.o	\
		 $(COMMON_PATH)/cJSON.o	\
		 $(COMMON_PATH)/deal_mysql.o   \
		 $(COMMON_PATH)/des.o	\
		 $(COMMON_PATH)/md5.o	\
		 $(COMMON_PATH)/redis_op.o	\
		 $(COMMON_PATH)/util_cgi.o
	$(CC) $^   $(LIBPATH) $(LIBS) -o $@

#生成所有的.o文件
%.o:%.c
	$(CC) -c  $(CFLAGS) $< -o $@ $(INCLUDEFLAGS)
#清除项目
clean:
	-rm -rf *.o $(target) $(CGI_SRC_PATH)/*.o $(COMMON_PATH)/*.o
#声明伪文件
.PHONY:clean ALL
