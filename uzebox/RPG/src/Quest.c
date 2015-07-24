#define MAX_QUEST_DATA 16

typedef uint8_t QuestState;

typedef struct Quest_s Quest;
typedef QuestState (*QuestHandler)(struct Quest_s*);

enum
{
	QS_Init,
	QS_Shutdown,
	QS_Idle,
	QS_Completed,
	DefaultQuestStateCount
};

struct Quest_s
{
	QuestHandler handler;
	QuestState state;
	uint8_t data[MAX_QUEST_DATA];
};

Quest CurrentQuest;

void Quest_Start(QuestHandler handler)
{
	CurrentQuest.handler = handler;
	CurrentQuest.state = QS_Init;
}

void Quest_Update(Quest* quest)
{
	quest->state = quest->handler(quest);
}


// Test quest
enum TestQuest_State
{
	TestQuest_StateDialog1 = DefaultQuestStateCount,
	TestQuest_Waiting,
	TestQuest_StateDialog2
};

struct TestQuest_s
{
	uint16_t counter;
};

QuestState TestQuest(Quest* quest)
{
	struct TestQuest_s* questData = (struct TestQuest_s*) quest->data;
	
	switch(quest->state)
	{
		case QS_Init:
		{
			questData->counter = 30;
			return TestQuest_StateDialog1;
		}
		case TestQuest_StateDialog1:
		{
			Dialog_Print("Hello world!");
			return TestQuest_Waiting;
		}
		case TestQuest_Waiting:
		{
			if(questData->counter > 0)
				questData->counter --;
			else 
				return TestQuest_StateDialog2;
		}
		case TestQuest_StateDialog2:
		{
			Dialog_Print("Quest complete!");
			return QS_Completed;
		}
	}
	return quest->state;
}
