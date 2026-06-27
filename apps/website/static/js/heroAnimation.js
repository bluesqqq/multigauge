import { lerp } from "/static/js/utils.js"

function variableSmoothstep(x, k = 1) {
  x = Math.max(0, Math.min(1, x)); // Clamp to [0, 1]
  if (k <= 0) return x; // Linear
  if (k === 1) return x * x * (3 - 2 * x); // Standard smoothstep
  if (k === 2) return x * x * x * (x * (x * 6 - 15) + 10); // Smootherstep

  // Generalized: raise x to a power based on k
  return Math.pow(x, k) / (Math.pow(x, k) + Math.pow(1 - x, k));
}


document.addEventListener('DOMContentLoaded', function() {
    const canvas = document.getElementById('heroBackground');
    const context = canvas.getContext('2d');

    function resizeCanvasToElement() {
        canvas.width = canvas.clientWidth;
        canvas.height = canvas.clientHeight;
    }

    resizeCanvasToElement();

    // Watch for changes to the canvas element size
    const resizeObserver = new ResizeObserver(resizeCanvasToElement);
    resizeObserver.observe(canvas);
    let startTime = performance.now();

    const centerRadius = 200;

    function degToRad(degrees) {
        return degrees * (Math.PI / 180);
    }

    function diamond(ctx, x, y, radius) {
        context.beginPath();
        context.lineTo(x - radius, y);
        context.lineTo(x, y + radius);
        context.lineTo(x + radius, y);
        context.lineTo(x, y - radius);
        context.closePath();

        context.fill();
        context.stroke();
    }

    function thickLine(context, x0, y0, x1, y1, thickness) {
        context.beginPath();
        context.lineTo(x0, y0);
        context.lineTo(x1, y1);
        context.lineWidth = thickness + lineWidth * 2;
        context.stroke();
        context.strokeStyle = "black";
        context.lineWidth = thickness;
        context.stroke();
        context.strokeStyle = "white";
        context.lineWidth = lineWidth;
    }

    let modifier = 0;
    const lineWidth = 4;

    function movingTickArc(context, time, x, y, or, ir, startAngle, endAngle, ticks, speed = 1) {
        startAngle = degToRad(startAngle);
        endAngle = degToRad(endAngle);

        const interval = 1 / (ticks - 1);
        const offset = (time/1000) % 1;
        const threshold = degToRad(1); // 1 degree in radians
        const centerRadius = (or + ir) / 2;

        for (let i = 0; i < ticks; i++) {
            const angle = lerp(startAngle, endAngle, ((i + offset) * interval));
            if (angle > endAngle) continue;

            const distToStart = Math.abs(angle - startAngle);
            const distToEnd = Math.abs(angle - endAngle);
            const minDist = Math.min(distToStart, distToEnd);

            // Compute interpolation factor (0 at > threshold, 1 at 0 distance)
            const factor = Math.max(0, 1 - minDist / threshold);

            const outer = lerp(or, centerRadius, factor);
            const inner = lerp(ir, centerRadius, factor);

            context.beginPath();
            context.lineTo(x + Math.cos(angle) * outer, y + Math.sin(angle) * outer);
            context.lineTo(x + Math.cos(angle) * inner, y + Math.sin(angle) * inner);
            context.stroke();
        }
    }

    function line(context, x0, y0, x1, y1) {
        context.beginPath();
        context.lineTo(x0, y0);
        context.lineTo(x1, y1);
        context.stroke();
    }

    function horizon(context, width, height, time, horizonHeight) {
        line(context, 0, horizonHeight, width, horizonHeight);

        const interval = width / 20;
        const offset = ((time/1000) % 1);
        for (let i = 0; i < 20; i++) {
            const position = (i + 1 - offset) * interval;

            const otherPosition = position + (position - (width / 2)) * 2;

            line(context, position, horizonHeight, otherPosition, height);
        }
    }

    const chassisImage = new Image();
    chassisImage.src = "/static/vector/vehicle.svg";

    const wheelsImg = new Image();
    wheelsImg.src = "/static/vector/vehicle-wheels.svg";

    // Your drawing code here
    function draw() {
        const time = performance.now() - startTime;

        modifier = lerp(modifier, 1, 0.02);
        const newModifier = modifier + Math.sin(time * 0.001) * 0.1;

        const smoothVal = variableSmoothstep(time/700, 4);
        const width = canvas.width;
        const height = canvas.height;

        const centerX = width / 2;
        const centerY = height / 2;

        const horizonHeight = height - 200;

        context.fillStyle = "black";
        context.fillRect(0, 0, width, height);
        context.lineCap = "round";

        context.lineWidth = lineWidth;
        context.strokeStyle = "white";
        
        const endAngleGauge = -135 * newModifier;

        context.beginPath();
        context.arc(centerX, centerY, centerRadius, degToRad(0), degToRad(endAngleGauge), true);
        context.lineTo(centerX + Math.cos(degToRad(endAngleGauge)) * (centerRadius + 100), centerY + Math.sin(degToRad(endAngleGauge)) * (centerRadius + 100));
        context.stroke();

        const otherEnd = 90 + 45 * newModifier;
        context.beginPath();
        context.arc(centerX, centerY, centerRadius, degToRad(90), degToRad(90 + 45 * newModifier));
        context.lineTo(centerX + Math.cos(degToRad(otherEnd)) * (centerRadius + 100), centerY + Math.sin(degToRad(otherEnd)) * (centerRadius + 100));
        context.stroke();
        
        diamond(context, centerX, centerY + centerRadius, 30 * smoothVal);

        movingTickArc(context, time, centerX, centerY, centerRadius + 30, centerRadius + 10, endAngleGauge, -45, 10);

        diamond(context, centerX + centerRadius, centerY, 30 * smoothVal);
        
        // Draw tick marks
        const ticks = 10;
        const startAngle = -10;
        const endAngle = -10 + 110 * newModifier;

        for (let i = 0; i < ticks; i++) {
            const angle = lerp(startAngle, endAngle, i/(ticks - 1));

            thickLine(context, Math.cos(degToRad(angle)) * 200, Math.sin(degToRad(angle)) * 200, Math.cos(degToRad(angle)) * 240, Math.sin(degToRad(angle)) * 240, 12);
        }

        horizon(context, width, height, time, horizonHeight);

        if (chassisImage.complete && wheelsImg.complete) {
            const scale = 4;
            const imgWidth = chassisImage.naturalWidth * scale;
            const imgHeight = chassisImage.naturalHeight * scale;

            const chassisRotation = degToRad((1 - newModifier) * 10);

            const x = (centerX - centerRadius * 2) * newModifier - imgWidth;
            const y = horizonHeight - imgHeight + 50;

            const rotationOffsetY = 30; // pixels below center

            context.save();
            context.translate(x + imgWidth / 2, y + imgHeight / 2 + rotationOffsetY);
            context.rotate(chassisRotation);
            context.drawImage(chassisImage, -imgWidth / 2, -imgHeight / 2 - rotationOffsetY, imgWidth, imgHeight);
            context.restore();


            // 🚫 DO NOT rotate wheels
            context.drawImage(wheelsImg, x, y, imgWidth, imgHeight);
        }

        requestAnimationFrame(draw);
    }

    draw();
});