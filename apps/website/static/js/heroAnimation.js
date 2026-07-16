import { lerp, variableSmoothstep, degToRad } from "/static/js/utils.js"

const MOBILE_MEDIA_QUERY = "(max-width: 1024px)";
const SE_SQRT_2 = Math.SQRT1_2;
const COLORS = {
    black: "#000000",
    white: "#ffffff",
};

function mixColor(start, end, amount) {
    const parse = (hex) => {
        const normalized = hex.replace("#", "");
        return [
            parseInt(normalized.slice(0, 2), 16),
            parseInt(normalized.slice(2, 4), 16),
            parseInt(normalized.slice(4, 6), 16),
        ];
    };

    const [sr, sg, sb] = parse(start);
    const [er, eg, eb] = parse(end);

    return `rgb(${Math.round(lerp(sr, er, amount))}, ${Math.round(lerp(sg, eg, amount))}, ${Math.round(lerp(sb, eb, amount))})`;
}

class Hero {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext("2d");
        this.startTime = performance.now();
        this.modifier = 0;
        this.mobileMediaQuery = window.matchMedia(MOBILE_MEDIA_QUERY);
        const hero = canvas.closest(".hero");
        this.heroCopy = hero?.querySelector(".hero-copy") ?? null;
        this.heroTitle = hero?.querySelector(".hero-title") ?? null;
        this.heroTagline = hero?.querySelector(".hero-tagline") ?? null;

        this.lineWidth = 2;
        this.width = 0;
        this.height = 0;
        this.frameId = null;

        this.openAmount = 0;
        this.isLoaded = document.readyState === "complete";
        this.inkColor = COLORS.black;
        this.geometry = {
            centerX: 0,
            centerY: 0,
            horizonHeight: 0,
            centerRadius: 0,
        };
        
        this.resizeObserver = new ResizeObserver(() => this.resize());

        if (!this.isLoaded) {
            window.addEventListener("load", () => {
                this.isLoaded = true;
            }, { once: true });
        }
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
        this.updateLayout(rect.width, rect.height);
    }

    updateLayout(width, height) {
        if (!this.heroCopy || !this.heroTitle || !this.heroTagline) return;

        if (this.mobileMediaQuery.matches) {
            this.setMobileCopyStyles(height);
            return;
        }

        this.setDesktopCopyStyles(width, height);
    }

    resetTextStyles() {
        for (const element of [this.heroTitle, this.heroTagline]) {
            element.style.position = "";
            element.style.left = "";
            element.style.top = "";
            element.style.right = "";
            element.style.transform = "";
            element.style.margin = "";
            element.style.width = "";
            element.style.maxWidth = "";
        }
    }

    setMobileCopyStyles(height) {
        this.heroCopy.style.position = "absolute";
        this.heroCopy.style.left = "0";
        this.heroCopy.style.right = "0";
        this.heroCopy.style.top = `${Math.round(height * 0.5)}px`;
        this.heroCopy.style.width = "100%";
        this.heroCopy.style.height = `${Math.round(height * 0.25)}px`;
        this.heroCopy.style.display = "flex";
        this.heroCopy.style.flexDirection = "column";
        this.heroCopy.style.alignItems = "center";
        this.heroCopy.style.justifyContent = "flex-start";
        this.heroCopy.style.gap = "0.25rem";
        this.heroCopy.style.pointerEvents = "none";

        this.resetTextStyles();

        this.heroTitle.style.position = "static";
        this.heroTagline.style.position = "static";
        this.heroTitle.style.margin = "0";
        this.heroTagline.style.margin = "0";
    }

    setDesktopCopyStyles(width, height) {
        const centerX = width / 2;
        const centerY = height * 0.4;
        const radius = height * 0.25;

        this.heroCopy.style.position = "absolute";
        this.heroCopy.style.inset = "0";
        this.heroCopy.style.pointerEvents = "none";
        this.heroCopy.style.display = "block";

        this.resetTextStyles();

        this.heroTitle.style.position = "absolute";
        this.heroTitle.style.left = "auto";
        this.heroTitle.style.right = `${width - (centerX - radius)}px`;
        this.heroTitle.style.top = `${centerY}px`;
        this.heroTitle.style.transform = "translateY(-50%)";
        this.heroTitle.style.margin = "0";

        const taglineAnchorX = centerX + radius * SE_SQRT_2;
        const taglineAnchorY = centerY + radius * SE_SQRT_2;

        this.heroTagline.style.position = "absolute";
        this.heroTagline.style.left = `${taglineAnchorX}px`;
        this.heroTagline.style.top = `${taglineAnchorY}px`;
        this.heroTagline.style.transform = "translateY(-50%)";
        this.heroTagline.style.margin = "0";
    }

    // DRAWING

    line(x0, y0, x1, y1) {
        const ctx = this.ctx;

        ctx.beginPath();
        ctx.lineTo(x0, y0);
        ctx.lineTo(x1, y1);
        ctx.stroke();
    }

    diamond(x, y, radius, openAmount = 1, startVertex = "left", direction = "clockwise") {
        const ctx = this.ctx;
        const sideLength = radius * Math.SQRT2;
        const totalLength = sideLength * 4;
        const drawLength = totalLength * openAmount;
        const points = {
            left: [x - radius, y],
            top: [x, y - radius],
            right: [x + radius, y],
            bottom: [x, y + radius],
        };
        const pathOrder = direction === "clockwise"
            ? ["left", "bottom", "right", "top"]
            : ["left", "top", "right", "bottom"];
        const startIndex = pathOrder.indexOf(startVertex);
        const orderedVertices = startIndex < 0
            ? pathOrder
            : pathOrder.slice(startIndex).concat(pathOrder.slice(0, startIndex));

        ctx.save();
        ctx.strokeStyle = this.inkColor;
        ctx.setLineDash([drawLength, totalLength]);
        ctx.beginPath();
        ctx.moveTo(...points[orderedVertices[0]]);
        for (let i = 1; i < orderedVertices.length; i++) {
            ctx.lineTo(...points[orderedVertices[i]]);
        }
        ctx.closePath();

        ctx.fillStyle = COLORS.black;
        ctx.fill();
        ctx.stroke();
        ctx.restore();
    }

    drawOpeningArc(
        time,
        startAngle,
        endAngle,
        innerRadius,
        outerRadius,
        counterclockwise,
        diamondRadius = 0,
        tickIntervalDeg = 10,
        tickCoverage = 0.75,
        tickOffsetDeg = 0,
        tickDirection = "clockwise",
        diamondStartVertex = "left",
        diamondDirection = "clockwise"
    ) {
        const ctx = this.ctx;
        const centerX = this.geometry.centerX;
        const centerY = this.geometry.centerY;

        const arcLength = innerRadius * Math.abs(endAngle - startAngle);
        const lineLength = outerRadius - innerRadius;
        const totalLength = arcLength + lineLength;
        const drawLength = totalLength * this.openAmount;

        ctx.save();
        ctx.strokeStyle = this.inkColor;
        ctx.setLineDash([drawLength, totalLength]);

        ctx.beginPath();
        ctx.arc(centerX, centerY, innerRadius, startAngle, endAngle, counterclockwise);
        ctx.lineTo(
            centerX + Math.cos(endAngle) * outerRadius,
            centerY + Math.sin(endAngle) * outerRadius
        );
        ctx.stroke();

        if (diamondRadius > 0) {
            this.diamond(
                centerX + Math.cos(startAngle) * innerRadius,
                centerY + Math.sin(startAngle) * innerRadius,
                diamondRadius,
                this.openAmount,
                diamondStartVertex,
                diamondDirection
            );
        }

        ctx.setLineDash([]);
        this.drawTickMarksOnArc(
            time,
            centerX,
            centerY,
            innerRadius + 10 + 10 * this.openAmount,
            innerRadius + 10,
            startAngle,
            endAngle,
            tickIntervalDeg,
            tickCoverage,
            tickOffsetDeg,
            tickDirection
        );

        ctx.restore();
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

    drawTickMarksOnArc(time, x, y, outerRadius, innerRadius, startAngle, endAngle, tickIntervalDeg, tickCoverage, tickOffsetDeg, tickDirection) {
        const ctx = this.ctx;

        const start = startAngle;
        const end = endAngle;
        const tickInterval = degToRad(tickIntervalDeg);
        const tickStep = tickDirection === "counterclockwise" ? -tickInterval : tickInterval;
        const phaseOffset = degToRad(tickOffsetDeg);
        const sweepEnd = lerp(end, start, tickCoverage);
        const sweepLength = Math.abs(sweepEnd - end);
        const centerRadius = (outerRadius + innerRadius) / 2;

        if (sweepLength <= 0) return;

        const sweepMin = Math.min(end, sweepEnd);
        const sweepMax = Math.max(end, sweepEnd);
        const tickPhase = phaseOffset + (tickDirection === "counterclockwise" ? -1 : 1) * ((time / 1000) % 1) * tickInterval;
        const firstTick = tickDirection === "counterclockwise"
            ? tickPhase + Math.floor((sweepMax - tickPhase) / tickInterval) * tickInterval
            : tickPhase + Math.ceil((sweepMin - tickPhase) / tickInterval) * tickInterval;

        for (
            let signedAngle = firstTick;
            tickDirection === "counterclockwise" ? signedAngle >= sweepMin : signedAngle <= sweepMax;
            signedAngle += tickStep
        ) {
            if (signedAngle < sweepMin || signedAngle > sweepMax) continue;

            const progress = tickDirection === "counterclockwise"
                ? (sweepMax - signedAngle) / sweepLength
                : (signedAngle - sweepMin) / sweepLength;
            const clamped = Math.max(0, Math.min(1, progress));
            const factor = clamped < 0.05
                ? 1 - (clamped / 0.05)
                : (clamped - 0.05) / 0.95;

            const outer = lerp(outerRadius, centerRadius, factor);
            const inner = lerp(innerRadius, centerRadius, factor);

            ctx.beginPath();
            ctx.lineTo(x + Math.cos(signedAngle) * outer, y + Math.sin(signedAngle) * outer);
            ctx.lineTo(x + Math.cos(signedAngle) * inner, y + Math.sin(signedAngle) * inner);
            ctx.stroke();
        }
    }

    decorator(time) {
        const newModifier = this.modifier + Math.sin(time * 0.001) * 0.1;
        const smoothVal = variableSmoothstep(time / 700, 4);
        const radius = this.geometry.centerRadius;
        const diamondRadius = radius * smoothVal * 0.1;

        this.drawOpeningArc(
            time,
            degToRad(0),
            degToRad(-135 * newModifier),
            radius * 0.8,
            radius * 1.0,
            true,
            diamondRadius,
            10,
            0.75,
            0,
            "clockwise",
            "top",
            "counterclockwise"
        );
        this.drawOpeningArc(
            time,
            degToRad(90),
            degToRad(135 * newModifier),
            radius * 0.8,
            radius * 1.0,
            false,
            diamondRadius,
            10,
            0.75,
            0,
            "counterclockwise",
            "left",
            "counterclockwise"
        );
    }

    updateGeometry(width, height, isMobile, navbarHeight) {
        const centerX = width / 2;
        const centerY = isMobile ? (height / 2 + navbarHeight) / 2 : height * 0.4;
        const centerRadius = isMobile ? (height / 2 - navbarHeight) / 2 : height * 0.25;

        this.geometry = {
            centerX,
            centerY,
            centerRadius,
            horizonHeight: height * 0.75,
        };
    }

    draw = () => {
        const ctx = this.ctx;
        const time = performance.now() - this.startTime;
        const { width, height } = this;

        const isMobile = this.mobileMediaQuery.matches;
        const navbarHeight = 60;

        this.openAmount = lerp(this.openAmount, this.isLoaded ? 1 : 0, 0.01);
        this.updateGeometry(width, height, isMobile, navbarHeight);
        this.modifier = lerp(this.modifier, 1, 0.02);
        this.inkColor = mixColor(COLORS.black, COLORS.white, this.openAmount);

        ctx.fillStyle = COLORS.black;
        ctx.fillRect(0, 0, width, height);

        ctx.lineCap = "round";
        ctx.lineWidth = this.lineWidth;
        ctx.strokeStyle = this.inkColor;
        ctx.fillStyle = COLORS.black;

        this.decorator(time);
        this.horizon(time, this.geometry.horizonHeight);

        this.frameId = requestAnimationFrame(this.draw);
    };
}

document.addEventListener("DOMContentLoaded", () => {
    const canvas = document.getElementById("heroBackground");

    if (!canvas) return;

    const hero = new Hero(canvas);
    hero.start();
});
