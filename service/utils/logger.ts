const isProduction = process.env.NODE_ENV === 'production';

function createTimestamp() {
    return new Date()
        .toLocaleString('zh-CN', {
            year: 'numeric',
            month: '2-digit',
            day: '2-digit',
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit',
            hour12: false,
        })
        .replace(/\//g, '-');
}

function formatMessage(module: string | undefined, msg: string) {
    const prefix = module ? `[${module}] ` : '';
    return `${prefix}${msg}`;
}

class PrettyLogger {
    info(msg: string, module?: string) {
        console.log(`[${createTimestamp()}] ${formatMessage(module, msg)}`);
    }
    error(msg: string, module?: string) {
        console.error(`[${createTimestamp()}] ${formatMessage(module, msg)}`);
    }
    warn(msg: string, module?: string) {
        console.warn(`[${createTimestamp()}] ${formatMessage(module, msg)}`);
    }
    debug(msg: string, module?: string) {
        if (!isProduction) console.debug(`[${createTimestamp()}] ${formatMessage(module, msg)}`);
    }
    child(_bindings: { module: string }) {
        return this;
    }
}

export const logger = isProduction ? new PrettyLogger() : new PrettyLogger();

export const createLogger = (name: string) => {
    return {
        info: (msg: string) => logger.info(msg, name),
        error: (msg: string) => logger.error(msg, name),
        warn: (msg: string) => logger.warn(msg, name),
        debug: (msg: string) => logger.debug(msg, name),
    };
};
