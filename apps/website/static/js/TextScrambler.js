export class TextScrambler {
  constructor(el, text, duration = 1000, flickerInterval = 100) {
    this.el = el;
    this.text = text;
    this.duration = duration;
    this.flickerInterval = flickerInterval;
    this.chars = "@#%*+=:.";
    this.queue = [];
    this.lastFlicker = 0;
    this.currentRandom = [];
    this.startTime = null;
    this.animate = this.animate.bind(this);
    this.prepareQueue();
    requestAnimationFrame(this.animate);
  }

  prepareQueue() {
    const length = this.text.length;
    for (let i = 0; i < length; i++) {
      this.queue.push({
        from: this.chars[Math.floor(Math.random() * this.chars.length)],
        to: this.text[i],
        startTime: Math.random() * this.duration * 0.4,
        endTime: Math.random() * this.duration * 0.4 + this.duration * 0.6
      });
      this.currentRandom[i] = this.queue[i].from;
    }
    this.el.textContent = '';
  }

  animate(currentTime) {
    if (!this.startTime) this.startTime = currentTime;
    const elapsed = currentTime - this.startTime;

    // Only flicker new characters every flickerInterval ms
    if (elapsed - this.lastFlicker >= this.flickerInterval) {
      for (let i = 0; i < this.queue.length; i++) {
        const q = this.queue[i];
        if (elapsed >= q.startTime && elapsed < q.endTime) {
          this.currentRandom[i] = this.chars[Math.floor(Math.random() * this.chars.length)];
        }
      }
      this.lastFlicker = elapsed;
    }

    let output = '';
    let done = 0;

    for (let i = 0; i < this.queue.length; i++) {
      const { to, startTime, endTime } = this.queue[i];
      if (elapsed >= endTime) {
        output += to;
        done++;
      } else if (elapsed >= startTime) {
        output += this.currentRandom[i];
      } else {
        output += ' ';
      }
    }

    this.el.textContent = output;

    if (done < this.queue.length) {
      requestAnimationFrame(this.animate);
    }
  }
}

// Auto-run on elements with .scramble
document.addEventListener("DOMContentLoaded", () => {
  document.querySelectorAll(".scramble").forEach(el => {
    const text = el.getAttribute("data-text") || el.textContent;
    const duration = parseInt(el.getAttribute("data-duration")) || 0;
    new TextScrambler(el, text, duration);
  });
});