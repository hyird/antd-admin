import type { JwtPayload } from '../modules/system/auth/auth.types.js';

export interface AppEnv {
    Variables: {
        jwt: JwtPayload;
    };
}
