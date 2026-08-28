#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#define TEMP_MAX 35.0f   // 温度上限
#define TEMP_MIN 10.0f    // 温度下限
#define HUMI_MAX 85.0f   // 湿度上限
#define HUMI_MIN 40.0f   // 湿度下限
#define CONTINUOUS_ABNORMAL_THRESHOLD 3
//添加了温湿度宏定义

//数据结构定义
//试验田块信息(使用动态数组管理){
typedef struct Field{
	int id;//田块id,从1开始自动分配;
	char name[30];//田块名称,如东区试验田1号
	char manager[20];//负责人姓名
	int sensor_count;//该田块已安装的传感器数量
}Field;
//传感器数据记录(使用链表管理)
typedef struct SensorRecord{
	int field_id;//对应的田块id
	char timestamp[20];//记录时间“2025-04-10 14:30”
	float temperature;//温度值，单位°C
	float humidity;//湿度值，单位%
	int is_abnormal;//0-正常，1-异常
	struct SensorRecord* next;//指向下一条记录
}SensorRecord;
//系统告警信息(使用链表管理)
typedef struct Alert{
	int field_id;//发生告警的田块id
	char alert_time[20];//告警时间
	char alert_type[30];//告警类型
	char description[100];//告警详细描述
	int severity;//严重程度，1-轻微，2-中等，3-严重
	struct Alert* next;//指向下一条告警
}Alert;
//监测系统主结构
typedef struct MonitorSystem{
	Field** fields;//田块指针数组(二重指针)
	int field_count;//当前田块数量
	int field_capacity;//数组容量，初始为10
	SensorRecord *record_head;//传感器记录链表头指针
	Alert *alert_head;//告警信息链表头指针
	int next_field_id;//下一个可用田块的id，用于保证每个新创建的田块都有一个唯一且连续的id
}MonitorSystem;
//趋势枚举
typedef enum {
    TREND_STABLE,    // 平稳
    TREND_RISING,    // 上升
    TREND_FALLING,   // 下降
    TREND_UNDEFINED  // 无足够数据无法判断
} TrendType;
//告警等级枚举
typedef enum {
    ALERT_LEVEL_1 = 1, // 严重告警
    ALERT_LEVEL_2,     // 中度告警
    ALERT_LEVEL_3      // 轻度告警
} AlertLevel;
//声明全局变量
static MonitorSystem* sys = NULL;
void check_continuous_abnormal_alerts(MonitorSystem* sys, int field_id);
//函数功能实现
//系统管理函数
MonitorSystem* create_monitor_system(void){
	//创建系统
	MonitorSystem* sys=(MonitorSystem*)malloc(sizeof(MonitorSystem));
	if(sys==NULL){
		perror("Failed to allocate MonitorSystem");
		return NULL;
	}
	//初始化所有成员
	sys->fields=NULL;
	sys->field_count=0;
	sys->field_capacity=10;
	sys->record_head=NULL;
	sys->alert_head=NULL;
	sys->next_field_id=1;
	//初始化田块指针数组
	sys->fields=(Field**)malloc(sys->field_capacity*sizeof(Field*));
	if(sys->fields==NULL){
		perror("Failed to allocate fields array");
		free(sys);
		sys=NULL;
	}
	
	//初始化每个指针
	for(int i=0;i<sys->field_capacity;i++){
		sys->fields[i]=NULL;
	}
	return sys;
}
void destroy_monitor_system(MonitorSystem* sys){
	if(sys==NULL){
		perror("Failed to allocate MonitorSystem");
		return;
	}
	if(sys->fields!=NULL){
		for(int i=0;i<sys->field_capacity;i++){
			if(sys->fields[i]!=NULL){
				free(sys->fields[i]);
				sys->fields[i]=NULL;
			}
			
			
	}
		free(sys->fields);
		sys->fields=NULL;
	}
	
	Alert* curr=sys->alert_head;
	while(curr!=NULL){
		Alert* temp = curr;
		curr=curr->next;
		free(temp);
	}
	
	SensorRecord* cur=sys->record_head;
	while(cur!=NULL){
		SensorRecord* temp =cur;
		cur=cur->next;
		free(temp);
	}
	
	free(sys);
	sys=NULL;
}
void display_main_menu(void){
	for(int i=40;i>=0;i--){printf("=");}printf("\n");
	for(int i=8;i>=0;i--){printf(" ");}
	printf("校园实验田环境监测系统\n");
	for(int i=40;i>=0;i--){printf("=");}printf("\n");
	printf("1.实验田管理\n");
	printf("2.传感器数据管理\n");
	printf("3.数据查询与统计\n");
	printf("4.告警管理\n");
	printf("5.系统信息\n");
	printf("0.退出系统\n");
	for(int i=40;i>=0;i--){printf("=");}printf("\n");
	printf("请选择操作(0-5):");
}
void expand_field_array(MonitorSystem* sys){
	if(sys==NULL){
		perror("Failed to allocate MonitorSystem");
		return;
	}
	int new_capacity=sys->field_capacity*2;
	Field** temp_fields=(Field**)realloc(sys->fields,new_capacity*sizeof(Field*));
	if(temp_fields!=NULL){
		sys->fields=temp_fields;
		sys->field_capacity=new_capacity;
	for(int i=sys->field_count;i<sys->field_capacity;i++){
		sys->fields[i]=NULL;
	}
	printf("田块数组扩容成功！原容量：%d-> 新容量：%d\n",sys->field_capacity / 2, new_capacity);
	}else{
		perror("错误：田块数组扩容失败（realloc 内存分配失败）");
	}
} 
//实验田管理函数
int add_field(MonitorSystem *sys, const char *name, const char *manager){
	if(sys==NULL){
		perror("Failed to allocate MonitorSystem");
		return 0;
	}
//	 分配新实验田内存并赋值
	Field* new_field=(Field*)malloc(sizeof(Field));
	new_field->id=sys->next_field_id;
	strcpy(new_field->name,name);
	strcpy(new_field->manager,manager);
	new_field->sensor_count=3;
//如果数组容量不足则自动翻倍
	if(sys->field_capacity==sys->field_count){expand_field_array(sys);}
//	 加入数组
	sys->fields[sys->field_count]=new_field;
	sys->field_count++;
	sys->next_field_id++;
	return 0;
}
void display_all_fields(MonitorSystem* sys){
	if(sys==NULL){
		perror("Failed to allocate MonitorSystem");
		return ;
		}	
	for(int i=40;i>=0;i--){printf("=");}printf("\n");
		for(int i=8;i>=0;i--){printf(" ");}
		printf("所有实验田块\n");
		for(int i=40;i>=0;i--){printf("=");}printf("\n");
		printf("ID     名 称         负责人     传感器数\n");
		for(int i=40;i>=0;i--){printf("-");}printf("\n");
		for(int i=0;i<sys->field_count;i++){
			printf("%d.   %s    %s         %d\n",sys->fields[i]->id,sys->fields[i]->name,sys->fields[i]->manager,sys->fields[i]->sensor_count);
		}
		for(int i=40;i>=0;i--){printf("=");}printf("\n");
}
Field* find_field_by_id(MonitorSystem* sys,int field_id){
	
	if (sys == NULL) {
	    perror("Failed to allocate MonitorSystem");
	    return NULL;
	}
	if (field_id <= 0) {
	    printf("错误：田块ID必须为正整数！\n");
	    return NULL;
	}
	for(int i = 0; i < sys->field_count; i++){
		if (sys->fields[i] == NULL) {
		    continue;
		}
		if (sys->fields[i]->id == field_id) {
		    return sys->fields[i];
		}
	}
	printf("提示：未找到ID为%d的实验田块！\n", field_id);
	return NULL;
}
int delete_field(MonitorSystem* sys,int field_id){
	    // 1. 参数校验
	if (sys == NULL) {
	    perror("Failed to allocate MonitorSystem");
	    return -1;
	}
	if (field_id <= 0) {
	    printf("错误：田块ID必须为正整数！\n");
	    return -1;
	}
	
	    // 2. 找到对应田块的数组下标
	    int target_index = -1;
	    for (int i = 0; i < sys->field_count; i++) {
	        if (sys->fields[i] != NULL && sys->fields[i]->id == field_id) {
	            target_index = i;
	            break;
	        }
	    }
	    if (target_index == -1) {
	        printf("未找到ID为%d的田块\n", field_id);
	        return -1;
	    }
	
	    // 3. 释放目标田块的内存
    if (sys->record_head != NULL) {
        SensorRecord *prev_rec = NULL;
        SensorRecord *curr_rec = sys->record_head;
        while (curr_rec != NULL) {
            if (curr_rec->field_id == field_id) {
                // 处理头节点删除
                if (prev_rec == NULL) {
                    sys->record_head = curr_rec->next;
                    free(curr_rec);
                    curr_rec = sys->record_head;
                }
                // 处理中间/尾节点删除
                else {
                    prev_rec->next = curr_rec->next;
                    free(curr_rec);
                    curr_rec = prev_rec->next;
                }
            } else {
                prev_rec = curr_rec;
                curr_rec = curr_rec->next;
            }
        }
    }
    if (sys->alert_head != NULL) {
        Alert *prev_alt = NULL;
        Alert *curr_alt = sys->alert_head;
        while (curr_alt != NULL) {
            if (curr_alt->field_id == field_id) {
                // 处理头节点删除
                if (prev_alt == NULL) {
                    sys->alert_head = curr_alt->next;
                    free(curr_alt);
                    curr_alt = sys->alert_head;
                }
                // 处理中间/尾节点删除
                else {
                    prev_alt->next = curr_alt->next;
                    free(curr_alt);
                    curr_alt = prev_alt->next;
                }
            } else {
                prev_alt = curr_alt;
                curr_alt = curr_alt->next;
            }
        }
    }    
	
	    // 4. 移动后续田块填充空缺（关键：避免数组留空）
	    for (int i = target_index; i < sys->field_count - 1; i++) {
	        sys->fields[i] = sys->fields[i + 1];
	    }
	    // 最后一个位置置空
	    sys->fields[sys->field_count - 1] = NULL;
	
	    // 5. 维护田块总数
	    sys->field_count--;
	
	    printf("提示：已成功删除ID=%d的实验田，且清理其所有关联监测记录和告警记录！\n", field_id);
	    return 0;
}
void search_fields_by_name(MonitorSystem* sys,const char* keyword){
	
	if(keyword==NULL||strlen(keyword)==0){
		printf("错误：搜索关键词不能为空！\n");
		return;
	}
	if (sys == NULL) {
	    perror("Failed to allocate MonitorSystem");
	    return ;
	}
	
	int found=0;
	for(int i = 0; i < sys->field_count; i++){
		if (sys->fields[i]==NULL) {
		    continue;
		}
		if (strstr(sys->fields[i]->name , keyword)!=NULL) {
			Field* p=sys->fields[i];
		    printf("找到匹配实验田,ID为%d, 名称为%s 管理人为%s 传感器数为%d\n",p->id,p->name,p->manager,p->sensor_count);
		    found=1;
		}
	}
		if(found){return;}
		else{
		printf("提示：未找到名称为%s的实验田块！\n", keyword);
		return ;}
}
void search_fields_by_manager(MonitorSystem* sys,const char* manager_name){
	int found=0;
	if (sys == NULL) {
	    perror("Failed to allocate MonitorSystem");
	    return ;
	}
		
	for(int i = 0; i < sys->field_count; i++){
		if (sys->fields[i]==NULL) {
		    continue;
		}
		if (strstr(sys->fields[i]->manager , manager_name)!=NULL) {
			Field* p=sys->fields[i];
		    printf("找到匹配实验田,ID为%d, 名称为%s, 管理人为%s, 传感器数为%d\n",p->id,p->name,p->manager,p->sensor_count);
		    found=1;
		}
	}
		if(found){return;}
		else{
		printf("提示：未找到名称为%s的实验田块！\n", manager_name);
		return ;}
}
//传感器数据管理函数
// 辅助函数：比较两个时间字符串（新的返回1，旧的返回-1，相同返回0）
int compare_timestamp(const char *ts1, const char *ts2) {
	
    struct tm tm1 = {0}, tm2 = {0};
    // 手动解析 "YYYY-MM-DD HH:MM" 格式的时间字符串
    if (sscanf(ts1, "%d-%d-%d %d:%d", &tm1.tm_year, &tm1.tm_mon, &tm1.tm_mday, &tm1.tm_hour, &tm1.tm_min) != 5) {
        return 0; // 解析失败默认相等
    }
    if (sscanf(ts2, "%d-%d-%d %d:%d", &tm2.tm_year, &tm2.tm_mon, &tm2.tm_mday, &tm2.tm_hour, &tm2.tm_min) != 5) {
        return 0;
    }
    // 转换为 tm 结构体的标准格式（年份减 1900，月份减 1）
    tm1.tm_year -= 1900;
    tm1.tm_mon -= 1;
    tm2.tm_year -= 1900;
    tm2.tm_mon -= 1;
    
    time_t t1 = mktime(&tm1), t2 = mktime(&tm2);
    return (t1 > t2) ? 1 : (t1 < t2) ? -1 : 0;
}
int add_sensor_record(MonitorSystem* sys,int field_id,float temperature,float humidity){
	//合法性校验
	if(sys==NULL||field_id<=0){
		printf("参数错误：系统指针为空或田块ID无效！\n");
	}
	if(find_field_by_id(sys,field_id)==NULL){
		printf("添加失败：ID=%d的田块不存在！\n", field_id);
		return -1;
	}
	
	SensorRecord* new_SensorRecord=(SensorRecord*)malloc(sizeof(SensorRecord));
	if(new_SensorRecord==NULL){
		perror("malloc SensorRecord失败");
		return -1;
	}
	new_SensorRecord->field_id=field_id;
	new_SensorRecord->temperature=temperature;
	new_SensorRecord->humidity=humidity;
	time_t now =time(NULL);
	struct tm* tm_info=localtime(&now);
	strftime(new_SensorRecord->timestamp,sizeof(new_SensorRecord->timestamp),"%Y-%m-%d %H:%M", tm_info);
	new_SensorRecord->is_abnormal=0;
	if (temperature < 10 || temperature > 35 || humidity < 40 || humidity > 85){
		new_SensorRecord->is_abnormal=1;
		printf("温湿度异常！已标记该记录，将自动生成告警\n");
	} 
	//链表头插法
	new_SensorRecord->next=sys->record_head;//新节点指向原来的第一个节点
	sys->record_head=new_SensorRecord;//头节点指向新节点
	printf("添加记录成功\n");
	printf("时间：%s | 田块ID：%d | 温度：%.1f℃ | 湿度：%.1f%% | 状态：%s\n",
	        new_SensorRecord->timestamp, field_id, temperature, humidity,
	        new_SensorRecord->is_abnormal ? "异常" : "正常");
	check_continuous_abnormal_alerts(sys, field_id);
	return 0;
}
void display_latest_records(MonitorSystem* sys,int field_id,int count){
		if (sys == NULL) {
	        perror("Failed to allocate MonitorSystem");
	        return;
	    }
	    if (field_id <= 0) {
	        printf("错误：实验田ID必须为正整数！\n");
	        return;
	    }
	    if (count <= 0) {
	        printf("错误：展示条数必须为正整数！\n");
	        return;
	    }
	    if (sys->record_head == NULL) {
	        printf("提示：系统中暂无传感器记录！\n");
	        return;
	    }
	    //提取出「目标实验田的专属数据」
	SensorRecord* filtered =NULL;
	int filtered_count=0;
	for(SensorRecord* p=sys->record_head;p;p=p->next){
		if(p->field_id==field_id){
			SensorRecord *new_node =(SensorRecord*)malloc(sizeof(SensorRecord));
			if (!new_node) { perror("内存分配失败"); return; }
			memcpy(new_node, p, sizeof(SensorRecord));
			new_node->next = filtered;
			filtered = new_node;
			filtered_count++;
		}
	}
	if (filtered_count == 0) {
	    printf("该实验田暂无记录！\n");
	    return;
	}
	//如果下一条记录新，则与上一条记录交换
	for (int i = 0; i < filtered_count-1; i++) {
	        int swapped = 0;
	        for (SensorRecord *c = filtered; c && c->next; c = c->next) {
	            if (compare_timestamp(c->timestamp, c->next->timestamp) < 0) {
	                // 交换两个节点的数据
	                SensorRecord temp = *c;
	                memcpy(c, c->next, sizeof(SensorRecord));
	                memcpy(c->next, &temp, sizeof(SensorRecord));
	                swapped = 1;
	            }
	        }
	        if (!swapped) break;//如果没交换，说明可以提前结束冒泡
	    }
	printf("========== 实验田ID:%d 最新%d条记录 ==========\n", field_id, count);
	printf("时间                温度(℃)  湿度(%%)  状态\n");
	for (SensorRecord *q = filtered; q && count-- > 0; q = q->next) {
	    printf("%s  %.1f      %.1f     %s\n", q->timestamp, q->temperature,
	            q->humidity, q->is_abnormal ? "异常" : "正常");
	}  
	while (filtered) {
	    SensorRecord *temp = filtered;
	    filtered = filtered->next;
	    free(temp);
	}//让 filtered先指向下一块，然后放掉上一块，如果下一块还有接着放 
}
void display_field_all_records(MonitorSystem* sys,int field_id){
	if (sys == NULL) {
	    perror("Failed to allocate MonitorSystem");
	    return;
	}
	if (field_id <= 0) {
	    printf("错误：实验田ID必须为正整数！\n");
	    return;
	}
	if (sys->record_head == NULL) {
	    printf("提示：系统中暂无任何传感器记录！\n");
	    return;
	}
	SensorRecord* filtered = NULL;
	int filtered_count = 0;    
	for (SensorRecord* p = sys->record_head; p != NULL; p = p->next) {
	    if (p->field_id == field_id) {     
	SensorRecord* new_node = (SensorRecord*)malloc(sizeof(SensorRecord));
	
	        if (new_node == NULL) {
	            perror("malloc 临时记录节点失败"); 
			while (filtered != NULL) {
			        SensorRecord* temp = filtered;
			        filtered = filtered->next;
			        free(temp);
			    }
			        return;
			    }
	memcpy(new_node, p, sizeof(SensorRecord));
	        new_node->next = filtered;
	        filtered = new_node;
	        filtered_count++;
	    }
	}
	if (filtered_count == 0) {
        printf("提示：ID为%d的实验田暂无传感器记录！\n", field_id);
        return;
    }
	for (int i = 0; i < filtered_count - 1; i++) {
        int swapped = 0;
        for (SensorRecord* c = filtered; c != NULL && c->next != NULL; c = c->next) {
            if (compare_timestamp(c->timestamp, c->next->timestamp) < 0) {
                // 交换两个节点的内容（无需调整指针，仅交换数据）
                SensorRecord temp = *c;
                memcpy(c, c->next, sizeof(SensorRecord));
                memcpy(c->next, &temp, sizeof(SensorRecord));
                swapped = 1;
            }
        }
        if (!swapped) break; // 无交换则排序完成，提前退出
    }
	printf("========== 实验田ID:%d 所有传感器记录（共%d条） ==========\n", field_id, filtered_count);
	    printf("时间                温度(℃)  湿度(%%)  状态\n");
	    printf("------------------------------------------------\n");
	    for (SensorRecord* q = filtered; q != NULL; q = q->next) {
	        printf("%s  %.1f      %.1f     %s\n", 
	               q->timestamp, 
	               q->temperature, 
	               q->humidity, 
	               q->is_abnormal ? "异常" : "正常");
	    }
	    printf("================================================\n");
	
	    // 6. 释放临时链表的内存（避免内存泄漏）
	    while (filtered != NULL) {
	        SensorRecord* temp = filtered;
	        filtered = filtered->next;
	        free(temp);
	    }
	}
void delete_old_records(MonitorSystem* sys,int days){
    if (sys == NULL) {
        perror("Failed to allocate MonitorSystem");
        return;
    }
    if (days <= 0) {
        printf("错误：删除天数需为正整数！\n");
        return;
    }
    if (sys->record_head == NULL) {
        printf("提示：系统中无传感器记录，无需删除！\n");
        return;
    }
	time_t now = time(NULL);
    time_t cutoff = now - (days * 24 * 60 * 60); // 转换为秒数
    struct tm cutoff_tm = *localtime(&cutoff);
    
    SensorRecord *current = sys->record_head;
    SensorRecord *prev = NULL;
    while (current != NULL) {
        // 解析当前记录的时间戳为 tm 结构体
        struct tm record_tm = {0};
        if (sscanf(current->timestamp, "%d-%d-%d %d:%d", 
            &record_tm.tm_year, &record_tm.tm_mon, &record_tm.tm_mday,
            &record_tm.tm_hour, &record_tm.tm_min) != 5) {
            printf("警告：记录时间戳格式错误，跳过该记录！\n");
            prev = current;
            current = current->next;
            continue;
        }
        // 转换为 tm 标准格式（年份减1900，月份减1）
        record_tm.tm_year -= 1900;
        record_tm.tm_mon -= 1;
        record_tm.tm_sec = 0; // 补全秒数（原记录无秒数）
        record_tm.tm_isdst = -1; // 自动识别夏令时

        // 3. 比较记录时间与截止时间
        time_t record_time = mktime(&record_tm);
        if (record_time < cutoff) {
            // 需删除当前节点
            SensorRecord *to_delete = current;
            if (prev == NULL) {
                // 删除头节点
                sys->record_head = current->next;
            } else {
                // 删除中间/尾节点
                prev->next = current->next;
            }
            current = current->next; // 先移动指针，再释放内存
            free(to_delete);
            to_delete = NULL;
        } else {
            // 记录未过期，继续遍历
            prev = current;
            current = current->next;
        }
    }

    printf("成功删除 %d 天前的所有传感器记录！\n", days);
}
int get_record_count(MonitorSystem* sys,int field_id){
	if(sys==NULL){
		perror("Failed to allocate MonitorSystem");
		return 0;
	}
	if(field_id<=0){
		printf("错误：实验田ID必须为正整数！\n");
		return 0;
	}
	if(find_field_by_id(sys,field_id)==NULL){
		printf("错误：ID=%d的实验田不存在",field_id);
		return 0;
	}
	if (sys->record_head == NULL) {
	    printf("提示：系统中暂无任何传感器记录！\n");
	    return 0;
	}
	int count =0;
	SensorRecord* p=sys->record_head;
	while(p!=NULL){
		if(p->field_id==field_id){
			count++;
		}
		p=p->next;
	}
	return count;
}
float calculate_avg_temperature(MonitorSystem* sys,int field_id){
    if (sys == NULL) {
        perror("Failed to allocate MonitorSystem");
        return -1.0f; // 返回异常值标识
    }
    if (field_id <= 0) {
        printf("错误：实验田ID必须为正整数！\n");
        return -1.0f;
    }
    if (sys->record_head == NULL) {
        printf("提示：系统中无任何传感器记录，无法计算平均温度！\n");
        return 0.0f;
    }
    if (find_field_by_id(sys, field_id) == NULL) {
        printf("错误：ID=%d的实验田不存在，无法计算平均温度！\n",
               field_id);
        return -1.0f;
    }	
	float total_temp = 0.0f;
	int valid_count = 0;
	SensorRecord* current = sys->record_head;
	
    while (current != NULL) {
        if (current->field_id == field_id) {
            total_temp += current->temperature;
            valid_count++;
        }
        current = current->next;
    }
    if (valid_count == 0) {
        printf("提示：ID=%d的实验田无传感器记录，平均温度为0！\n",
               field_id);
        return 0.0f;
    }

    // 4. 计算并返回平均温度（保留1位小数）
    float avg_temp = total_temp / valid_count;
    printf("ID=%d的实验田平均温度：%.1f℃（共%d条有效记录）\n",
           field_id, avg_temp, valid_count);
    return avg_temp;		
}
//数据分析函数
void find_abnormal_records(MonitorSystem* sys,int field_id){
    // 1. 入参合法性校验
    if (sys == NULL) {
        perror("Failed to allocate MonitorSystem");
        return;
    }
    if (field_id <= 0) {
        printf("错误：实验田ID必须为正整数！\n");
        return;
    }
    if (sys->record_head == NULL) {
        printf("提示：系统中暂无任何传感器记录！\n");
        return;
    }
    // 2. 校验实验田是否存在
    Field* target_field = find_field_by_id(sys, field_id);
    if (target_field == NULL) {
        return; // find_field_by_id已输出错误提示，直接返回
    }

    // 3. 遍历链表筛选异常记录
    SensorRecord* current = sys->record_head;
    int abnormal_count = 0; // 异常记录计数
    printf("\n========== 实验田ID:%d 异常记录查询结果 ==========\n", field_id);
    printf("时间                温度(℃)  湿度(%%)  异常原因\n");
    printf("------------------------------------------------\n");

    while (current != NULL) {
        if (current->field_id == field_id && current->is_abnormal == 1) {
            abnormal_count++;
            // 分析异常原因
            char reason[50] = "";
            if (current->temperature < 10) {
                strcat(reason, "温度过低;");
            } else if (current->temperature > 35) {
                strcat(reason, "温度过高;");
            }
            if (current->humidity < 40) {
                strcat(reason, "湿度过低;");
            } else if (current->humidity > 85) {
                strcat(reason, "湿度过高;");
            }
            // 去除末尾的分号
            if (reason[strlen(reason)-1] == ';') {
                reason[strlen(reason)-1] = '\0';
            }
            // 打印异常记录
            printf("%s  %.1f      %.1f     %s\n", 
                   current->timestamp, 
                   current->temperature, 
                   current->humidity, 
                   reason);
        }
        current = current->next;
    }

    // 4. 输出统计结果
    if (abnormal_count == 0) {
        printf("                    暂无异常记录\n");
    } else {
        printf("------------------------------------------------\n");
        printf("总计找到 %d 条异常记录\n", abnormal_count);
        if(abnormal_count>=3){
			printf("连续3条异常:持续异常告警(3级)");
			
		}
    }
    printf("================================================\n");	
}
void find_extreme_values(MonitorSystem* sys,int field_id){
    // 1. 入参合法性校验
    if (sys == NULL) {
        perror("Failed to allocate MonitorSystem");
        return;
    }
    if (field_id <= 0) {
        printf("错误：实验田ID必须为正整数！\n");
        return;
    }
    if (sys->record_head == NULL) {
        printf("提示：系统暂无任何传感器记录，无极值可查！\n");
        return;
    }
    // 校验实验田是否存在
    Field* target_field = find_field_by_id(sys, field_id);
    if (target_field == NULL) {
        return; // find_field_by_id已输出错误提示，直接返回
    }

    // 2. 初始化极值变量（用第一个有效记录初始化）
    float max_temp = -100.0f, min_temp = 100.0f;
    float max_hum = 0.0f, min_hum = 100.0f;
    char max_temp_ts[20] = "", min_temp_ts[20] = "";
    char max_hum_ts[20] = "", min_hum_ts[20] = "";
    int has_record = 0; // 标记是否有该实验田的记录

    // 3. 遍历链表查找极值
    SensorRecord* current = sys->record_head;
    while (current != NULL) {
        if (current->field_id == field_id) {
            has_record = 1; // 确认有该实验田记录
            
            // 温度极值判断
            if (current->temperature > max_temp) {
                max_temp = current->temperature;
                strcpy(max_temp_ts, current->timestamp);
            }
            if (current->temperature < min_temp) {
                min_temp = current->temperature;
                strcpy(min_temp_ts, current->timestamp);
            }
            
            // 湿度极值判断
            if (current->humidity > max_hum) {
                max_hum = current->humidity;
                strcpy(max_hum_ts, current->timestamp);
            }
            if (current->humidity < min_hum) {
                min_hum = current->humidity;
                strcpy(min_hum_ts, current->timestamp);
            }
        }
        current = current->next;
    }

    // 4. 输出结果
    if (!has_record) {
        printf("提示：ID=%d的实验田暂无传感器记录，无极值可查！\n", field_id);
        return;
    }

    printf("\n========== 实验田ID:%d 极值查询结果 ==========\n", field_id);
    printf("┌──────────┬─────────────┬─────────────┬─────────────┐\n");
    printf("│ 类型     │ 最大值       │ 最大值时间   │ 最小值       │ 最小值时间   │\n");
    printf("├──────────┼─────────────┼─────────────┼─────────────┤\n");
    printf("│ 温度(℃)  │ %.1f        │ %s │ %.1f        │ %s │\n", 
           max_temp, max_temp_ts, min_temp, min_temp_ts);
    printf("│ 湿度(%%)  │ %.1f        │ %s │ %.1f        │ %s │\n", 
           max_hum, max_hum_ts, min_hum, min_hum_ts);
    printf("└──────────┴─────────────┴─────────────┴─────────────┘\n");	
}
void analyze_trend(MonitorSystem* sys,int field_id){
    // 1. 入参合法性校验
    if (sys == NULL) {
        perror("Failed to allocate MonitorSystem");
        return;
    }
    if (field_id <= 0) {
        printf("错误：实验田ID必须为正整数！\n");
        return;
    }
    if (sys->record_head == NULL) {
        printf("提示：系统暂无任何传感器记录，无法进行趋势分析！\n");
        return;
    }
    // 校验实验田是否存在
    Field* target_field = find_field_by_id(sys, field_id);
    if (target_field == NULL) {
        return;
    }

    // 2. 提取该实验田的所有有效记录（按时间戳顺序，链表已默认按采集时间排序）
    SensorRecord* current = sys->record_head;
    int record_count = 0;  // 记录总数
    float temp_array[100]; // 存储温度序列（支持最多100条记录，可按需扩展）
    float hum_array[100];  // 存储湿度序列
    char ts_array[100][20];// 存储对应时间戳
    while (current != NULL) {
        if (current->field_id == field_id) {
            temp_array[record_count] = current->temperature;
            hum_array[record_count] = current->humidity;
            strcpy(ts_array[record_count], current->timestamp);
            record_count++;
        }
        current = current->next;
    }

    // 3. 校验记录数量（至少2条记录才能判断趋势）
    if (record_count < 2) {
        printf("提示：ID=%d的实验田仅有%d条记录，数据量不足，无法判断趋势！\n", 
               field_id, record_count);
        return;
    }

    // 4. 趋势计算（基于线性拟合斜率，简化为首尾差值判断，兼顾效率和可读性）
    // 4.1 温度趋势计算
    float temp_diff = temp_array[record_count - 1] - temp_array[0]; // 末值 - 初值
    TrendType temp_trend;
    const float TREND_THRESHOLD = 0.5f; // 趋势阈值（变化小于0.5℃判定为平稳，可按需调整）
    if (fabs(temp_diff) < TREND_THRESHOLD) {
        temp_trend = TREND_STABLE;
    } else if (temp_diff > 0) {
        temp_trend = TREND_RISING;
    } else {
        temp_trend = TREND_FALLING;
    }

    // 4.2 湿度趋势计算
    float hum_diff = hum_array[record_count - 1] - hum_array[0];
    TrendType hum_trend;
    if (fabs(hum_diff) < TREND_THRESHOLD) { // 湿度采用相同阈值，可单独调整
        hum_trend = TREND_STABLE;
    } else if (hum_diff > 0) {
        hum_trend = TREND_RISING;
    } else {
        hum_trend = TREND_FALLING;
    }

    // 5. 格式化输出趋势结果
    printf("\n========== 实验田ID:%d 趋势分析结果 ==========\n", field_id);
    printf("数据时间范围：%s ~ %s\n", ts_array[0], ts_array[record_count - 1]);
    printf("有效记录数量：%d条\n", record_count);
    printf("┌──────────┬─────────────┬─────────────┬─────────────┐\n");
    printf("│ 类型     │ 初始值      │ 最终值      │ 趋势类型    │\n");
    printf("├──────────┼─────────────┼─────────────┼─────────────┤\n");
    
    // 温度结果输出
    printf("│ 温度(℃)  │ %.1f        │ %.1f        │ ", 
           temp_array[0], temp_array[record_count - 1]);
    switch (temp_trend) {
        case TREND_RISING:
            printf("上升（+%.1f℃） │\n", temp_diff);
            break;
        case TREND_FALLING:
            printf("下降（%.1f℃） │\n", temp_diff);
            break;
        case TREND_STABLE:
            printf("平稳（±%.1f℃） │\n", temp_diff);
            break;
        default:
            printf("无法判断     │\n");
            break;
    }

    // 湿度结果输出
    printf("│ 湿度(%%)  │ %.1f        │ %.1f        │ ", 
           hum_array[0], hum_array[record_count - 1]);
    switch (hum_trend) {
        case TREND_RISING:
            printf("上升（+%.1f%%） │\n", hum_diff);
            break;
        case TREND_FALLING:
            printf("下降（%.1f%%） │\n", hum_diff);
            break;
        case TREND_STABLE:
            printf("平稳（±%.1f%%） │\n", hum_diff);
            break;
        default:
            printf("无法判断     │\n");
            break;
    }
    printf("└──────────┴─────────────┴─────────────┴─────────────┘\n");

    // 6. 简单趋势可视化（字符画，提升可读性）
    printf("\n=== 温度趋势可视化（前10条记录）===\n");
    for (int i = 0; i < (record_count > 10 ? 10 : record_count); i++) {
        printf("%-16s | ", ts_array[i]);
        int star_count = (int)(temp_array[i] - 10); // 偏移量（假设温度≥10℃，可调整）
        for (int j = 0; j < star_count; j++) {
            printf("*");
        }
        printf(" (%.1f℃)\n", temp_array[i]);
    }

    printf("\n=== 湿度趋势可视化（前10条记录）===\n");
    for (int i = 0; i < (record_count > 10 ? 10 : record_count); i++) {
        printf("%-16s | ", ts_array[i]);
        int star_count = (int)(hum_array[i] / 2); // 缩放比例（湿度0~100%对应0~50个*）
        for (int j = 0; j < star_count; j++) {
            printf("*");
        }
        printf(" (%.1f%%)\n", hum_array[i]);
    }	
}
//告警管理函数
void check_continuous_abnormal_alerts(MonitorSystem* sys, int field_id) {
    // 1. 参数合法性校验
    if (sys == NULL || field_id <= 0) {
        printf("错误：参数非法，无法检测持续异常告警\n");
        return;
    }
    Field* target_field = find_field_by_id(sys, field_id);
    if (target_field == NULL) {
        printf("错误：实验田ID%d不存在\n", field_id);
        return;
    }

    // 2. 统计该实验田的连续异常数、累计异常数
    int abnormal_count = 0;    // 累计异常记录数
    int continuous_count = 0;  // 连续异常记录数
    int last_is_abnormal = 0;  // 上一条记录是否异常（标记连续状态）

    for (SensorRecord* p = sys->record_head; p != NULL; p = p->next) {
        if (p->field_id == field_id) { // 只统计当前实验田的记录
            if (p->is_abnormal == 1) { // 当前记录异常
                abnormal_count++;
                if (last_is_abnormal == 1) {
                    continuous_count++; // 连续异常，计数+1
                } else {
                    continuous_count = 1; // 首次异常，初始化连续计数
                }
                last_is_abnormal = 1;
            } else { // 当前记录正常，中断连续异常
                last_is_abnormal = 0;
            }
        }
    }

    // 3. 判断是否触发持续异常告警
    int trigger_alert = 0;
    char alert_subtype[20] = ""; // 告警子类型（连续/累计）
    if (continuous_count >= CONTINUOUS_ABNORMAL_THRESHOLD) {
        trigger_alert = 1;
        strcpy(alert_subtype, "连续异常");
    } 

    // 4. 触发告警：生成持续异常告警记录（级别为最高3级）
    if (trigger_alert) {
        Alert* new_alert = (Alert*)malloc(sizeof(Alert));
        if (new_alert == NULL) {
            perror("错误：分配告警内存失败");
            return;
        }

        // 填充告警信息
        new_alert->field_id = field_id;
        // 获取当前时间作为告警时间
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        strftime(new_alert->alert_time, sizeof(new_alert->alert_time), "%Y-%m-%d %H:%M", tm_info);
        
        strcpy(new_alert->alert_type, "持续环境异常");
        sprintf(new_alert->description,
                "实验田[%s]%s | 连续异常：%d条 累计异常：%d条",
                target_field->name, alert_subtype, continuous_count, abnormal_count);
        new_alert->severity = ALERT_LEVEL_3; // 持续异常设为最高级别
        new_alert->next = sys->alert_head;  // 插入告警链表头部
        sys->alert_head = new_alert;

        // 打印告警提示（控制台可视化）
        printf("??  告警触发：实验田ID%d %s！连续异常%d条/累计异常%d条\n",
               field_id, alert_subtype, continuous_count, abnormal_count);
    } else {
        // 未触发告警时的日志（可选）
        printf("??  实验田ID%d未触发持续异常告警 | 连续异常%d条/累计异常%d条\n",
               field_id, continuous_count, abnormal_count);
    }
}
void display_continuous_alerts(MonitorSystem* sys, int field_id) {
    // 1. 参数校验
    if (sys == NULL || field_id <= 0) {
        printf("错误：参数非法，无法查询持续异常告警\n");
        return;
    }
    if (sys->alert_head == NULL) {
        printf("提示：系统暂无任何告警记录\n");
        return;
    }

    // 2. 遍历告警链表，筛选指定实验田的持续异常告警
    printf("\n==================== 实验田ID:%d 持续异常告警记录 ====================\n", field_id);
    printf("%-18s %-15s %-6s %s\n", "告警时间", "告警类型", "级别", "详细描述");
    printf("----------------------------------------------------------------------\n");

    int found = 0; // 标记是否找到持续异常告警
    for (Alert* curr = sys->alert_head; curr != NULL; curr = curr->next) {
        // 筛选条件：实验田ID匹配 + 告警类型为“持续环境异常”
        if (curr->field_id == field_id && strstr(curr->alert_type, "持续环境异常") != NULL) {
            found = 1;
            printf("%-18s %-15s %d级      %s\n",
                   curr->alert_time,
                   curr->alert_type,
                   curr->severity,
                   curr->description);
        }
    }

    // 无记录时的提示
    if (!found) {
        printf("                          暂无持续异常告警记录                          \n");
    }
    printf("======================================================================\n\n");
}
void check_and_generate_alerts(MonitorSystem* sys,int field_id){
    // 1. 合法性校验
    if (sys == NULL) {
        perror("Failed to allocate MonitorSystem");
        return;
    }
    if (field_id <= 0) {
        printf("错误：实验田ID为非正数，无法生成告警！\n");
        return;
    }
    if (sys->record_head == NULL) {
        printf("提示：系统无传感器记录，无需生成告警！\n");
        return;
    }
    Field* target_field = find_field_by_id(sys, field_id);
    if (target_field == NULL) {
        return; // find_field_by_id已输出提示，直接返回
    }

    // 2. 遍历传感器记录，筛选未生成告警的异常记录
    SensorRecord* record_ptr = sys->record_head;
    int alert_count = 0; // 本次生成的告警数量
    while (record_ptr != NULL) {
        if (record_ptr->field_id == field_id && record_ptr->is_abnormal == 1) {
            // 3. 为每条异常记录生成告警节点
            Alert* new_alert = (Alert*)malloc(sizeof(Alert));
            if (new_alert == NULL) {
                perror("malloc Alert节点失败");
                return;
            }

            // 3.1 填充告警基础信息
            new_alert->field_id = field_id;
            strcpy(new_alert->alert_time, record_ptr->timestamp); // 告警时间复用异常记录时间

            // 3.2 确定告警类型和描述
            char temp_reason[50] = "";
            if (record_ptr->temperature < 10) {
                strcat(temp_reason, "温度过低;");
            } else if (record_ptr->temperature > 35) {
                strcat(temp_reason, "温度过高;");
            }
            if (record_ptr->humidity < 40) {
                strcat(temp_reason, "湿度过低;");
            } else if (record_ptr->humidity > 85) {
                strcat(temp_reason, "湿度过高;");
            }
            // 移除末尾分号
            if (temp_reason[strlen(temp_reason)-1] == ';') {
                temp_reason[strlen(temp_reason)-1] = '\0';
            }

            // 3.3 填充告警类型、描述、严重等级
            strcpy(new_alert->alert_type, "环境参数异常");
            sprintf(new_alert->description, 
                    "%s %s | 温度:%.1f℃ 湿度:%.1f%%",
                     target_field->name, temp_reason,
                    record_ptr->temperature, record_ptr->humidity);
            
            int param_error = 0;
            int temp_extreme =0, hum_extreme =0;
            if (record_ptr->temperature <TEMP_MIN || record_ptr->temperature>TEMP_MAX) param_error=2;
            if (record_ptr->temperature < 0 || record_ptr->temperature > 40) temp_extreme = 1;
            if (record_ptr->humidity <HUMI_MIN || record_ptr->humidity>HUMI_MAX) param_error=1;
            if (record_ptr->humidity < 20 || record_ptr->humidity > 95) hum_extreme = 1;
            int severity =0;
			if (temp_extreme || hum_extreme) {
			    
			    severity = ALERT_LEVEL_3;
			} else if (param_error == 2) {
			    
			    severity = ALERT_LEVEL_2;
			} else if (param_error == 1) {
			    
			    severity = ALERT_LEVEL_1;
			}
			new_alert->severity = severity;
            // 3.4 挂载告警节点到链表头部（头插法）
            new_alert->next = sys->alert_head;
            sys->alert_head = new_alert;

            alert_count++;
        }
        record_ptr = record_ptr->next;
    }

    // 4. 输出生成结果
    if (alert_count > 0) {
        printf("成功为实验田ID:%d 生成 %d 条告警记录！\n", field_id, alert_count);
    } else {
        printf("实验田ID:%d 暂无未处理的异常记录，无需生成告警！\n", field_id);
    }	
}
void display_all_alerts(MonitorSystem* sys){
    // 1. 入参合法性校验
    if (sys == NULL) {
        perror("Failed to allocate MonitorSystem");
        return;
    }
    if (sys->alert_head == NULL) {
        printf("提示：系统当前无任何告警记录！\n");
        return;
    }

    // 2. 临时链表存储告警（用于排序）
    Alert* sorted_alerts = NULL;
    int alert_count = 0;

    // 3. 复制原告警链表到临时链表
    Alert* original = sys->alert_head;
    while (original != NULL) {
        Alert* new_node = (Alert*)malloc(sizeof(Alert));
        if (new_node == NULL) {
            perror("malloc Alert节点失败");
            // 释放已分配的临时节点
            while (sorted_alerts != NULL) {
                Alert* temp = sorted_alerts;
                sorted_alerts = sorted_alerts->next;
                free(temp);
            }
            return;
        }
        
        memcpy(new_node, original, sizeof(Alert));
        new_node->next = sorted_alerts;
        sorted_alerts = new_node;
        alert_count++;
        original = original->next;
    }

    // 4. 按告警时间倒序排序（冒泡排序）
    for (int i = 0; i < alert_count - 1; i++) {
        int swapped = 0;
        Alert* current = sorted_alerts;
        while (current != NULL && current->next != NULL) {
            // 比较两个告警的时间，前者时间早于后者则交换
            if (compare_timestamp(current->alert_time, current->next->alert_time) < 0) {
                Alert temp = *current;
                memcpy(current, current->next, sizeof(Alert));
                memcpy(current->next, &temp, sizeof(Alert));
                swapped = 1;
            }
            current = current->next;
        }
        if (!swapped) break; // 无交换则排序完成
    }

    // 5. 展示告警信息
    printf("\n==================== 系统所有告警记录（共%d条） ====================\n", alert_count);
    printf("┌───────┬──────────────────┬──────────────┬──────────────────────────────────────┬─────────┐\n");
    printf("│ 实验田ID │ 告警时间         │ 告警类型     │ 告警描述                              │ 严重等级 │\n");
    printf("├───────┼──────────────────┼──────────────┼──────────────────────────────────────┼─────────┤\n");
    
    Alert* current = sorted_alerts;
    while (current != NULL) {
        // 转换严重等级为文字描述
        const char* severity_desc = "";
        switch (current->severity) {
            case 1: severity_desc = "一级"; break;
            case 2: severity_desc = "二级"; break;
            case 3: severity_desc = "三级"; break;
            default: severity_desc = "未知"; break;
        }
        
        // 格式化输出
        printf("│ %7d │ %16s │ %12s │ %40s │ %7s │\n",
               current->field_id,
               current->alert_time,
               current->alert_type,
               current->description,
               severity_desc);
        current = current->next;
    }
    printf("└───────┴──────────────────┴──────────────┴──────────────────────────────────────┴─────────┘\n");

    // 6. 释放临时排序链表的内存
    while (sorted_alerts != NULL) {
        Alert* temp = sorted_alerts;
        sorted_alerts = sorted_alerts->next;
        free(temp);
    }	
}
int resolve_alert(MonitorSystem* sys,int field_id,const char* alert_type){
    // 1. 入参合法性校验
    if (sys == NULL) {
        perror("Failed to allocate MonitorSystem");
        return -1;
    }
    if (field_id <= 0 || alert_type == NULL || strlen(alert_type) == 0) {
        printf("错误：实验田ID不合法或告警类型为空！\n");
        return -1;
    }
    if (sys->alert_head == NULL) {
        printf("提示：系统当前无任何告警记录，无需处理！\n");
        return 0;
    }

    // 2. 遍历告警链表，查找并删除匹配的告警
    Alert *current = sys->alert_head;
    Alert *prev = NULL;
    int resolved_count = 0; // 记录处理的告警数量

    while (current != NULL) {
        // 匹配实验田ID + 告警类型（模糊匹配，支持部分关键词）
        if (current->field_id == field_id && strstr(current->alert_type, alert_type) != NULL) {
            // 待删除节点暂存
            Alert *to_delete = current;
            
            // 调整链表指针
            if (prev == NULL) {
                // 删除头节点
                sys->alert_head = current->next;
            } else {
                // 删除中间/尾节点
                prev->next = current->next;
            }
            
            // 移动当前指针，释放待删除节点
            current = current->next;
            free(to_delete);
            to_delete = NULL;
            resolved_count++;
        } else {
            // 不匹配则向后遍历
            prev = current;
            current = current->next;
        }
    }

    // 3. 结果反馈
    if (resolved_count > 0) {
        printf("成功处理实验田ID:%d 下类型为「%s」的告警共 %d 条！\n", field_id, alert_type, resolved_count);
    } else {
        printf("未找到实验田ID:%d 下类型为「%s」的告警记录！\n", field_id, alert_type);
    }
    return resolved_count;	
}
int get_alert_count(MonitorSystem* sys,int field_id){
    // 1. 入参合法性校验
    if (sys == NULL) {
        perror("Failed to allocate MonitorSystem");
        return -1; // 返回-1标识错误
    }
    if (field_id <= 0) {
        printf("错误：实验田ID为非正数，无法统计告警数量！\n");
        return -1;
    }
    if (sys->alert_head == NULL) {
        printf("提示：系统当前无任何告警记录！\n");
        return 0;
    }

    // 2. 遍历告警链表，统计指定实验田的告警数
    int count = 0;
    Alert* current = sys->alert_head;
    while (current != NULL) {
        if (current->field_id == field_id) {
            count++;
        }
        current = current->next;
    }

    // 3. 输出统计结果并返回
    printf("实验田ID:%d 对应的告警记录总数：%d 条\n", field_id, count);
    return count;	
}
//辅助函数
void display_field_menu(void) {
    for(int i=40;i>=0;i--){printf("=");}printf("\n");
    for(int i=5;i>=0;i--){printf(" ");}
    printf("实验田管理\n");
    for(int i=40;i>=0;i--){printf("=");}printf("\n");
    printf("1. 添加新田块\n");
    printf("2. 显示所有田块\n");
    printf("3. 查找田块(按ID)\n");
    printf("4. 搜索田块(按名称)\n");
    printf("5. 搜索田块(按负责人)\n");
    printf("6. 删除田块\n");
    printf("0. 返回主菜单\n");
    for(int i=40;i>=0;i--){printf("=");}printf("\n");
    printf("请选择(0-6):");
}
void field_management(void){
	int sub_choice;
	
	while(1){
		display_field_menu();
		scanf("%d",&sub_choice);
		getchar();
		
	switch(sub_choice) {
		    case 1: 
		        printf("1. 添加新田块\n");
		        add_field(sys,"东区实验田1号","张三");
		        add_field(sys,"西区实验田2号","李四");
		        add_field(sys,"南区实验田3号","王五");
		        add_field(sys,"北区实验田4号","赵六");
		        add_field(sys,"中区实验田5号","钱七");
		        break;
		        
		    case 2: 
		        printf("2. 显示所有田块\n");
		        display_all_fields(sys);
		        break;
		        
		    case 3: {
		    	int field_id;
		        printf("3. 查找田块(按ID)\n");
		        printf("请输入要查找的ID");
				scanf("%d",&field_id);
				getchar();
				Field* p=find_field_by_id(sys,field_id);
				if(p!=NULL){
					printf("找到匹配实验田,ID为%d, 名称为%s 管理人为%s 传感器数为%d\n",p->id,p->name,p->manager,p->sensor_count);
					}else{
						printf("未找到ID为%d的田块！\n", field_id);
						}
		        
		        break;}
		    case 4: 
		    	char name[30];
		        printf("4. 搜索田块(按名称)\n");
		        printf("请输入要查找的田块名称(模糊匹配)；");
		        scanf("%s",name);
		        getchar();
		        search_fields_by_name(sys,name);
		        break;
		        
		    case 5: 
		    	char manager[20];
		        printf("5. 搜索田块(按负责人模糊匹配)\n");
		        printf("请输入要查找的田块管理员；");
		        scanf("%s",manager);
				getchar();
				search_fields_by_manager(sys,manager);
		        break;
		        
		    case 6:{
		    	int field_id;
		    	printf("6. 删除田块\n");
		    	printf("请输入要清除的田块：");
		    	scanf("%d",&field_id);
		    	getchar();
		    	delete_field(sys,field_id);
		    	printf("已清除ID为%d田块\n",field_id);
		    	break;}
		    case 0: 
		
		        return;
		        
		    default:
		        printf("\n请输入有效的数字!\n\n");
		        break;
			}
			
	}   
}
void display_SensorRecord_menu(void){
	for(int i=40;i>=0;i--){printf("=");}printf("\n");
    for(int i=5;i>=0;i--){printf(" ");}
    printf("传感器管理\n");
    for(int i=40;i>=0;i--){printf("=");}printf("\n");
    printf("1. 添加传感器记录 \n");
    printf("2. 显示最新记录\n");
    printf("3. 显示所有记录 \n");
    printf("4. 删除过期记录\n");
    printf("5. 获取记录数量\n");
    printf("6. 计算平均温度\n");
    printf("0. 返回主菜单\n");
    for(int i=40;i>=0;i--){printf("=");}printf("\n");
    printf("请选择(0-6):");
}
void SensorRecord_management(void){
	int sub_choice;
	while(1){
		display_SensorRecord_menu();
		scanf("%d",&sub_choice);
		getchar();
	switch(sub_choice) {
		case 1: {
			int field_id;
			float temperature,humidity;
			printf("1. 添加传感器记录 \n");	
			printf("请输入ID,温度,湿度\n");
			scanf("%d%f%f",&field_id,&temperature,&humidity);
			getchar();
			add_sensor_record(sys,field_id,temperature,humidity);       
	        break;}
				        
		case 2: {
			int field_id;
			int count;
			printf("2. 显示最新记录\n");
			printf("请输入ID,显示条数\n");
			scanf("%d%d",&field_id,&count);
			getchar();
			display_latest_records(sys,field_id,count);	        
			break;}
				        
		case 3: {
			int field_id;
			printf("3. 显示所有记录 \n");
			printf("请输入ID\n");
			scanf("%d",&field_id);
			getchar();
			display_field_all_records(sys,field_id);	        
			break;}
				        
		case 4: {
			int days;
			printf("4. 删除过期记录\n");
			printf("请输入保留最近几天的数据\n");
			scanf("%d",&days);
			getchar();
			delete_old_records(sys,days);	        
			break;}
				        
		case 5: {
			int field_id;
			int count;
			printf("5. 获取记录数量\n");
			printf("请输入ID\n");
			scanf("%d",&field_id);
			getchar();
			count=get_record_count(sys,field_id);
			printf("记录数量为%d\n",count);
			break;}
			
		case 6:{
			int field_id;
			printf("6. 计算平均温度\n");
			printf("请输入ID\n");
			scanf("%d",&field_id);
			getchar();
			calculate_avg_temperature(sys,field_id);
			break;}
				        
		case 0: 
				
			return;
				        
		default:
			printf("\n请输入有效的数字!\n\n");	        
			break;
					}
	}
}
void display_data_query(void){
		for(int i=40;i>=0;i--){printf("=");}printf("\n");
	    for(int i=5;i>=0;i--){printf(" ");}
	    printf("数据查询与统计\n");
	    for(int i=40;i>=0;i--){printf("=");}printf("\n");
	    printf("1. 查找异常记录 \n");
	    printf("2. 查找极值\n");
	    printf("3. 分析趋势 \n");
	    printf("0. 返回主菜单\n");
	    for(int i=40;i>=0;i--){printf("=");}printf("\n");
	    printf("请选择(0-3):");
}
void data_management(void){
	int sub_choice;
	while(1){
		display_data_query();
		scanf("%d",&sub_choice);
		getchar();
		switch(sub_choice){
			case 1:{
				int field_id;
				printf("1. 查找异常记录 \n");
				printf("请输入ID\n");
				scanf("%d",&field_id);
				getchar();
				find_abnormal_records(sys,field_id);
				break;}
			case 2:{
				int field_id;
				printf("2. 查找极值\n");
				printf("请输入ID\n");
				scanf("%d",&field_id);
				getchar();
				find_extreme_values(sys,field_id);
				break;}
			case 3:{
				int field_id;
				printf("3. 分析趋势 \n");
				printf("请输入ID\n");
				scanf("%d",&field_id);
				getchar();
				analyze_trend(sys,field_id);
				break;}
			case 0:
				
				return;
			default:
				printf("\n请输入有效的数字!\n\n");
				break;	
		}
	}
}
void display_alert(void){
	for(int i=40;i>=0;i--){printf("=");}printf("\n");
	    for(int i=5;i>=0;i--){printf(" ");}
		printf("告警管理\n");
		for(int i=40;i>=0;i--){printf("=");}printf("\n");
		printf("1. 检查并生成告警 \n");
		printf("2. 显示所有告警\n");
		printf("3. 处理告警 \n");
		printf("4. 统计告警数量 \n");
		printf("5. 显示连续告警\n");
		printf("0. 返回主菜单\n");
		for(int i=40;i>=0;i--){printf("=");}printf("\n");
		printf("请选择(0-4):");
}
void alert_management(void){
	int sub_choice;
	while(1){
		display_alert();
		scanf("%d",&sub_choice);
		getchar();
		switch(sub_choice){
			case 1:{
				int field_id;
				printf("1. 检查并生成告警 \n");
				printf("请输入ID\n");
				scanf("%d",&field_id);
				getchar();
				check_and_generate_alerts(sys,field_id);
				break;}
			case 2:
				printf("2. 显示所有告警\n");
				display_all_alerts(sys);
				break;
			case 3:{
				int field_id;
				char alert_type[20];
				printf("3. 处理告警 \n");
				printf("请输入ID,告警类型\n");
				scanf("%d%s",&field_id,alert_type);
				getchar();
				resolve_alert(sys,field_id,alert_type);
				break;}
			case 4:{
				int field_id;
				printf("4. 统计告警数量 \n");
				printf("请输入ID\n");
				scanf("%d",&field_id);
				getchar();
				get_alert_count(sys,field_id);
				break;}
			case 5:{
				int field_id;
				printf("5. 显示连续告警 \n");
				printf("请输入ID\n");
				scanf("%d",&field_id);
				getchar();
				display_continuous_alerts(sys,field_id);
				break;}
			case 0:
				
				return;
			default:
				printf("\n请输入有效的数字!\n\n");
				break;					
		}
	}
}
void display_system(void){
	for(int i=40;i>=0;i--){printf("=");}printf("\n");
		    for(int i=5;i>=0;i--){printf(" ");}
			printf("系统信息管理\n");
			for(int i=40;i>=0;i--){printf("=");}printf("\n");
			printf("1. 当前田块数量 \n");
			printf("2. 数组容量\n");
			printf("3. 版权信息\n");
			printf("0. 返回主菜单\n");
			for(int i=40;i>=0;i--){printf("=");}printf("\n");
			printf("请选择(0-3):");
}
void system_management(void){
	int sub_choice;
		while(1){
			display_system();
			scanf("%d",&sub_choice);
			getchar();
			switch(sub_choice){
				case 1:{
					printf("1. 当前田块数量为%d \n",sys->field_count);
					break;}
				case 2:
					printf("2. 数组容量为%d \n",sys->field_capacity);
					break;
				case 3:
					printf("计科252陆奕君\n");
					break;
				case 0:
					
					return;
				default:
					printf("\n请输入有效的数字!\n\n");
					break;					
			}
		}
}
int main(){
	sys = create_monitor_system();
	int main_choice;
	while(1){ 
		display_main_menu();
		scanf("%d", &main_choice);
		getchar();
		
	switch(main_choice) {
	    case 1: // 实验田管理
	        field_management();// 进入实验田管理子界面
	        break;
	        
	    case 2: // 传感器数据管理
	        SensorRecord_management();
	        break;
	        
	    case 3: // 数据查询统计
	        data_management();
	        break;
	        
	    case 4: // 告警管理
	        alert_management();
	        break;
	        
	    case 5: // 系统信息
	        system_management();
	        break;
	        
	    case 0: // 退出系统
	    
	        destroy_monitor_system(sys);
	        return 0;
	        
	    default:
	        printf("\n请输入有效的数字!\n\n");
	        break;
	}
	
	}
	
	
}
