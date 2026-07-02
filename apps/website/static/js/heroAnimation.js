import { lerp, variableSmoothstep, degToRad } from "/static/js/utils.js"

const MOBILE_MEDIA_QUERY = "(max-width: 720px)";

class Hero {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext("2d");
        this.startTime = performance.now();
        this.modifier = 0;
        this.mobileMediaQuery = window.matchMedia(MOBILE_MEDIA_QUERY);

        this.lineWidth = 2;
        this.centerRadius = 200;
        this.width = 0;
        this.height = 0;
        this.frameId = null;
        
        this.resizeObserver = new ResizeObserver(() => this.resize());
    }

    // LIFECYCLE

    start() {
        this.resize();
        this.resizeObserver.observe(this.canvas);
        this.frameId = requestAnimationFrame(this.draw);
    }

    stop() {
        cancelAnimationFrame(this.frameId);
        this.resizeObserver.disconnect();
    }

    resize() {
        const rect = this.canvas.getBoundingClientRect();
        const dpr = window.devicePixelRatio || 1;
        this.width = rect.width;
        this.height = rect.height;

        this.canvas.width = Math.round(rect.width * dpr);
        this.canvas.height = Math.round(rect.height * dpr);

        this.ctx.setTransform(dpr, 0, 0, dpr, 0, 0);

        const isMobile = this.mobileMediaQuery.matches;
        const centerY = isMobile ? rect.height * 0.30 : rect.height / 2;
        const centerRadius = isMobile
            ? Math.min(rect.height * 0.2, rect.width * 0.46)
            : Math.min(rect.height * 0.2, rect.width * 0.15);

        const hero = this.canvas.closest(".hero");
        if (hero) {
            hero.style.setProperty(
                "--hero-mobile-copy-top",
                isMobile
                    ? `${Math.round(centerY + centerRadius + rect.height * 0.035)}px`
                    : "0px"
            );
        }
    }

    // DRAWING

    line(x0, y0, x1, y1) {
        const ctx = this.ctx;

        ctx.beginPath();
        ctx.lineTo(x0, y0);
        ctx.lineTo(x1, y1);
        ctx.stroke();
    }

    diamond(x, y, radius) {
        const ctx = this.ctx;

        ctx.beginPath();
        ctx.lineTo(x - radius, y);
        ctx.lineTo(x, y + radius);
        ctx.lineTo(x + radius, y);
        ctx.lineTo(x, y - radius);
        ctx.closePath();

        ctx.fill();
        ctx.stroke();
    }

    horizon(time, horizonHeight) {
        const width = this.width;
        const height = this.height;

        this.line(0, horizonHeight, width, horizonHeight);

        const interval = 80;
        const count = Math.ceil(width / interval) + 2;
        const offset = (time / 1000) % 1;

        for (let i = 0; i < count; i++) {
            const position = (i + 1 - offset) * interval;
            const otherPosition = position + (position - width / 2) * 2;

            this.line(position, horizonHeight, otherPosition, height);
        }
    }

    movingTickArc(time, x, y, outerRadius, innerRadius, startAngle, endAngle, ticks) {
        const ctx = this.ctx;

        startAngle = degToRad(startAngle);
        endAngle = degToRad(endAngle);

        const interval = 1 / (ticks - 1);
        const offset = (time / 1000) % 1;
        const threshold = degToRad(1);
        const centerRadius = (outerRadius + innerRadius) / 2;

        for (let i = 0; i < ticks; i++) {
            const angle = lerp(startAngle, endAngle, (i + offset) * interval);

            if (angle > endAngle) continue;

            const distToStart = Math.abs(angle - startAngle);
            const distToEnd = Math.abs(angle - endAngle);
            const minDist = Math.min(distToStart, distToEnd);

            const factor = Math.max(0, 1 - minDist / threshold);

            const outer = lerp(outerRadius, centerRadius, factor);
            const inner = lerp(innerRadius, centerRadius, factor);

            ctx.beginPath();
            ctx.lineTo(x + Math.cos(angle) * outer, y + Math.sin(angle) * outer);
            ctx.lineTo(x + Math.cos(angle) * inner, y + Math.sin(angle) * inner);
            ctx.stroke();
        }
    }

    draw = () => {
        const ctx = this.ctx;
        const time = performance.now() - this.startTime;

        const width = this.width;
        const height = this.height;
        const isMobile = this.mobileMediaQuery.matches;

        const centerX = width / 2;
        const centerY = isMobile ? height * 0.30 : height / 2;
        const horizonHeight = height * 0.75;

        this.centerRadius = isMobile
            ? Math.min(height * 0.2, width * 0.46)
            : Math.min(height * 0.2, width * 0.15);

        this.modifier = lerp(this.modifier, 1, 0.02);

        const newModifier = this.modifier + Math.sin(time * 0.001) * 0.1;
        const smoothVal = variableSmoothstep(time / 700, 4);

        ctx.fillStyle = "black";
        ctx.fillRect(0, 0, width, height);

        ctx.lineCap = "round";
        ctx.lineWidth = this.lineWidth;
        ctx.strokeStyle = "white";
        ctx.fillStyle = "black";

        const endAngleGauge = -135 * newModifier;

        ctx.beginPath();
        ctx.arc(
            centerX,
            centerY,
            this.centerRadius,
            degToRad(0),
            degToRad(endAngleGauge),
            true
        );
        ctx.lineTo(
            centerX + Math.cos(degToRad(endAngleGauge)) * (this.centerRadius * 1.3),
            centerY + Math.sin(degToRad(endAngleGauge)) * (this.centerRadius * 1.3)
        );
        ctx.stroke();

        const otherEnd = 90 + 45 * newModifier;

        ctx.beginPath();
        ctx.arc(
            centerX,
            centerY,
            this.centerRadius,
            degToRad(90),
            degToRad(otherEnd)
        );
        ctx.lineTo(
            centerX + Math.cos(degToRad(otherEnd)) * (this.centerRadius * 1.3),
            centerY + Math.sin(degToRad(otherEnd)) * (this.centerRadius * 1.3)
        );
        ctx.stroke();

        this.diamond(centerX, centerY + this.centerRadius, this.centerRadius * smoothVal * 0.1);

        this.movingTickArc(
            time,
            centerX,
            centerY,
            this.centerRadius * 1.15,
            this.centerRadius * 1.05,
            endAngleGauge,
            -45,
            10
        );

        this.diamond(centerX + this.centerRadius, centerY, this.centerRadius * smoothVal * 0.1);

        this.horizon(time, horizonHeight);

        this.frameId = requestAnimationFrame(this.draw);
    };
}

document.addEventListener("DOMContentLoaded", () => {
    const canvas = document.getElementById("heroBackground");

    if (!canvas) return;

    const hero = new Hero(canvas);
    hero.start();
});
