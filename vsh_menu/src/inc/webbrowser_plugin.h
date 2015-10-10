// Mysis webbrowser_plugin.h v0.1
typedef struct
{
	int (*PluginWakeup)(int);
	int (*PluginWakeupWithUrl)(const char *);
	int (*PluginWakeupWithUrlAndRestriction)(const char *, int);
	int (*Shutdown)(void);
	int (*SetMotionType)(int);
	int (*DoUnk5)(char *);
	int (*PluginWakeupWithUrlString)(int *);
	int (*PluginWakeupWithSearchString)(int *);
	int (*PluginWakeupWithUrlAndExitHandler)(const char *, void *);
	int (*DoUnk9)(const char *, const char *);
	int (*UpdatePointerDisplayPos)(int, int, int);
} webbrowser_if; // Interface Id 1

webbrowser_if * webbrowser_interface;

typedef struct
{
	int (*Action)(const char *);
} webbrowser_act0_if; // Interface ACT0

webbrowser_act0_if * webbrowser_act0;
