import { create } from 'zustand';
import { persist } from 'zustand/middleware';

export const HOME_TAB = {
    key: '/home',
    title: '首页',
} as const;

export interface TabItem {
    key: string;
    title: string;
}

interface TabsState {
    tabs: TabItem[];
    activeKey: string;
}

interface TabsActions {
    addTab: (tab: TabItem) => void;
    removeTab: (key: string) => string | null;
    setActiveKey: (key: string) => void;
    clearTabs: () => void;
    setTabsState: (tabs: TabItem[], activeKey: string) => void;
}

export type TabsStore = TabsState & TabsActions;

const ensureHomeTab = (tabs: TabItem[]): TabItem[] => {
    const hasHome = tabs.some((t) => t.key === HOME_TAB.key);
    return hasHome ? tabs : [HOME_TAB, ...tabs];
};

export const useTabsStore = create<TabsStore>()(
    persist(
        (set, get) => ({
            tabs: [HOME_TAB],
            activeKey: HOME_TAB.key,

            addTab: (tab) => {
                const { tabs } = get();
                const exists = tabs.some((t) => t.key === tab.key);
                if (!exists) {
                    set({ tabs: [...tabs, tab] });
                }
                set({ activeKey: tab.key });
            },

            removeTab: (key) => {
                if (key === HOME_TAB.key) return null;

                const { tabs, activeKey } = get();
                const index = tabs.findIndex((t) => t.key === key);
                if (index === -1) return null;

                const newTabs = tabs.filter((t) => t.key !== key);
                let newActiveKey: string;

                if (activeKey === key) {
                    const newIndex = Math.max(0, Math.min(index, newTabs.length - 1));
                    newActiveKey = newTabs[newIndex]?.key ?? HOME_TAB.key;
                } else {
                    newActiveKey = activeKey;
                }

                set({ tabs: newTabs, activeKey: newActiveKey });
                return newActiveKey;
            },

            setActiveKey: (key) => {
                set({ activeKey: key });
            },

            clearTabs: () => {
                set({ tabs: [HOME_TAB], activeKey: HOME_TAB.key });
            },

            setTabsState: (tabs, activeKey) => {
                set({
                    tabs: ensureHomeTab(tabs),
                    activeKey: tabs.some((t) => t.key === activeKey) ? activeKey : HOME_TAB.key,
                });
            },
        }),
        {
            name: 'tabs-storage',
            partialize: (state) => ({
                tabs: ensureHomeTab(state.tabs),
                activeKey: state.activeKey,
            }),
        }
    )
);
