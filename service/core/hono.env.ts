import type { JwtPayload } from '@/modules/system/auth/auth.types';

export interface AppEnv {
    Variables: {
        jwt: JwtPayload;
    };
}
