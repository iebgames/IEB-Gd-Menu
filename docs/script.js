const selector = document.getElementById('versionSelector');
const trigger = selector.querySelector('.select-trigger span');
const btn = document.getElementById('finalDownload');
let selectedUrl = "";

selector.addEventListener('click', () => {
    selector.classList.toggle('active');
});

document.querySelectorAll('.option').forEach(opt => {
    opt.addEventListener('click', (e) => {
        const url = opt.getAttribute('data-url');
        const text = opt.innerText;
        
        trigger.innerText = text;
        selectedUrl = url;
        btn.disabled = false;
        
        // Extract version for button text
        const versionMatch = text.match(/v\d+\.\d+\.\d+/);
        if (versionMatch) {
            btn.innerText = `DOWNLOAD ${versionMatch[0]}.GEODE`;
        }
    });
});

document.addEventListener('click', (e) => {
    if (!selector.contains(e.target)) {
        selector.classList.remove('active');
    }
});

btn.addEventListener('click', () => {
    if (selectedUrl) window.location.href = selectedUrl;
});


// Particle Effect
const canvas = document.getElementById('particleCanvas');
const ctx = canvas.getContext('2d');
let particles = [];

function resize() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
}
window.addEventListener('resize', resize);
resize();

class Particle {
    constructor() {
        this.reset();
    }
    reset() {
        this.x = Math.random() * canvas.width;
        this.y = Math.random() * canvas.height;
        this.size = Math.random() * 2;
        this.speedX = (Math.random() - 0.5) * 0.5;
        this.speedY = (Math.random() - 0.5) * 0.5;
        this.alpha = Math.random();
    }
    update() {
        this.x += this.speedX;
        this.y += this.speedY;
        if (this.x < 0 || this.x > canvas.width || this.y < 0 || this.y > canvas.height) this.reset();
    }
    draw() {
        ctx.fillStyle = `rgba(139, 92, 246, ${this.alpha})`;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
        ctx.fill();
    }
}

for (let i = 0; i < 100; i++) particles.push(new Particle());

function animate() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    particles.forEach(p => {
        p.update();
        p.draw();
    });
    requestAnimationFrame(animate);
}
animate();
