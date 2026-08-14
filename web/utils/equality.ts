export function deepEqual(a: unknown, b: unknown): boolean {
    if (a === b) return true;
    if (typeof a !== typeof b) return false;
    if (a == null || b == null) return false;
    if (typeof a !== 'object') {
        return Number.isNaN(a) && Number.isNaN(b);
    }

    const objA = a as object;
    const objB = b as object;
    if (objA.constructor !== objB.constructor) return false;
    if (objA instanceof Date && objB instanceof Date) {
        return objA.getTime() === objB.getTime();
    }
    if (objA instanceof RegExp && objB instanceof RegExp) {
        return objA.source === objB.source && objA.flags === objB.flags;
    }
    if (objA instanceof Map && objB instanceof Map) {
        if (objA.size !== objB.size) return false;
        for (const [key, value] of objA) {
            if (!objB.has(key) || !deepEqual(value, objB.get(key))) {
                return false;
            }
        }
        return true;
    }
    if (objA instanceof Set && objB instanceof Set) {
        if (objA.size !== objB.size) return false;
        for (const value of objA) {
            let found = false;
            for (const bValue of objB) {
                if (deepEqual(value, bValue)) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }
    if (Array.isArray(objA) && Array.isArray(objB)) {
        if (objA.length !== objB.length) return false;
        return objA.every((item, index) => deepEqual(item, objB[index]));
    }

    const recordA = objA as Record<string, unknown>;
    const recordB = objB as Record<string, unknown>;
    const keysA = Object.keys(recordA);
    const keysB = Object.keys(recordB);
    if (keysA.length !== keysB.length) return false;
    return keysA.every((key) => key in recordB && deepEqual(recordA[key], recordB[key]));
}
