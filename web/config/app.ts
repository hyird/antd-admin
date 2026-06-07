const fallbackAppName = 'Enterprise Admin';
export const APP_NAME = import.meta.env.VITE_APP_NAME?.trim() || fallbackAppName;

export function getAppTitle(suffix?: string) {
    return suffix ? `${APP_NAME} - ${suffix}` : APP_NAME;
}
