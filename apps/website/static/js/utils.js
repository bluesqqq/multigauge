export function lerp(start, end, amount) {
    return start + (end - start) * amount;
}

export function degToRad(degrees) {
    return degrees * Math.PI / 180;
}

export function variableSmoothstep(x, k = 1) {
    x = Math.max(0, Math.min(1, x));

    if (k <= 0) return x;
    if (k === 1) return x * x * (3 - 2 * x);
    if (k === 2) return x * x * x * (x * (x * 6 - 15) + 10);

    return Math.pow(x, k) / (Math.pow(x, k) + Math.pow(1 - x, k));
}
