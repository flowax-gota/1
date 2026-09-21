/**
 * Electronic Sandbox - 3D Circuit Constructor
 * Web-based visualization using Three.js
 * 
 * This file provides the 3D rendering and interaction layer
 * that connects to the C++ simulation core via WebAssembly (future)
 * or runs a JavaScript simulation.
 */

// ============================================
// Main Game Class
// ============================================
class ElectronicSandboxGame {
    constructor() {
        this.canvas = document.getElementById('gameCanvas');
        this.scene = null;
        this.camera = null;
        this.renderer = null;
        this.components = [];
        this.wires = [];
        this.selectedComponent = null;
        this.currentTool = 'select';
        this.isPaused = false;
        this.lastFrameTime = 0;
        this.fps = 60;
        
        // Camera controls
        this.cameraAngle = { x: 0.5, y: 0.5 };
        this.cameraDistance = 10;
        this.cameraTarget = { x: 0, y: 0, z: 0 };
        
        this.init();
        this.setupEventListeners();
        this.animate();
    }
    
    init() {
        // Set canvas size
        this.resizeCanvas();
        
        // Create scene
        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(0x1a1a2e);
        
        // Create camera
        this.camera = new THREE.PerspectiveCamera(
            75,
            this.canvas.width / this.canvas.height,
            0.1,
            1000
        );
        this.updateCameraPosition();
        
        // Create renderer
        this.renderer = new THREE.WebGLRenderer({
            canvas: this.canvas,
            antialias: true
        });
        this.renderer.setSize(this.canvas.width, this.canvas.height);
        this.renderer.setPixelRatio(window.devicePixelRatio);
        
        // Add lights
        this.addLights();
        
        // Add grid helper
        this.addGridHelper();
        
        // Add example components
        this.addExampleComponents();
        
        console.log('Electronic Sandbox initialized');
    }
    
    addLights() {
        const ambientLight = new THREE.AmbientLight(0xffffff, 0.5);
        this.scene.add(ambientLight);
        
        const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
        directionalLight.position.set(10, 10, 5);
        this.scene.add(directionalLight);
        
        const pointLight = new THREE.PointLight(0x4fc3f7, 0.5);
        pointLight.position.set(0, 5, 0);
        this.scene.add(pointLight);
    }
    
    addGridHelper() {
        const gridHelper = new THREE.GridHelper(20, 20, 0x444444, 0x222222);
        this.scene.add(gridHelper);
        
        // Add axes helper
        const axesHelper = new THREE.AxesHelper(2);
        this.scene.add(axesHelper);
    }
    
    addExampleComponents() {
        // Create example AND gate
        const andGate = this.createComponent('AND', { x: 0, y: 0.5, z: 0 }, 0x2ecc71);
        this.components.push(andGate);
        
        // Create example OR gate
        const orGate = this.createComponent('OR', { x: 2, y: 0.5, z: 0 }, 0x3498db);
        this.components.push(orGate);
        
        // Create example XOR gate
        const xorGate = this.createComponent('XOR', { x: 4, y: 0.5, z: 0 }, 0xf1c40f);
        this.components.push(xorGate);
        
        // Create switch
        const switch1 = this.createComponent('SWITCH', { x: -2, y: 0.5, z: 0 }, 0xe74c3c);
        this.components.push(switch1);
        
        // Create LED
        const led = this.createComponent('LED', { x: 6, y: 0.5, z: 0 }, 0xe67e22);
        this.components.push(led);
        
        // Create wire between AND and OR
        this.createWire(andGate, orGate);
        
        this.updateStatistics();
    }
    
    createComponent(type, position, color) {
        const group = new THREE.Group();
        group.position.set(position.x, position.y, position.z);
        group.userData = {
            type: type,
            isComponent: true,
            id: 'comp_' + Date.now() + Math.random(),
            state: false
        };
        
        // Create component body based on type
        let geometry;
        const material = new THREE.MeshPhongMaterial({ 
            color: color,
            shininess: 100
        });
        
        switch(type) {
            case 'AND':
                geometry = new THREE.BoxGeometry(1, 0.8, 0.3);
                break;
            case 'OR':
                geometry = new THREE.BoxGeometry(1, 0.8, 0.3);
                break;
            case 'XOR':
                geometry = new THREE.BoxGeometry(1, 0.8, 0.3);
                break;
            case 'NOT':
                geometry = new THREE.BoxGeometry(0.6, 0.6, 0.3);
                break;
            case 'SWITCH':
                geometry = new THREE.BoxGeometry(0.5, 0.5, 0.5);
                break;
            case 'LED':
                geometry = new THREE.SphereGeometry(0.3, 16, 16);
                break;
            case 'MONITOR':
                geometry = new THREE.BoxGeometry(1.5, 1, 0.2);
                break;
            default:
                geometry = new THREE.BoxGeometry(0.8, 0.8, 0.8);
        }
        
        const mesh = new THREE.Mesh(geometry, material);
        group.add(mesh);
        
        // Add pins
        this.addPinsToComponent(group, type);
        
        // Add label
        this.addLabel(group, type);
        
        this.scene.add(group);
        return group;
    }
    
    addPinsToComponent(group, type) {
        const pinMaterial = new THREE.MeshPhongMaterial({ color: 0x888888 });
        const pinGeometry = new THREE.CylinderGeometry(0.05, 0.05, 0.2, 8);
        
        // Add input pins (left side)
        if (type !== 'SWITCH' && type !== 'LED') {
            const in1 = new THREE.Mesh(pinGeometry, pinMaterial);
            in1.rotation.x = Math.PI / 2;
            in1.position.set(-0.5, 0.2, 0);
            in1.userData = { isPin: true, direction: 'input', name: 'IN1' };
            group.add(in1);
            
            const in2 = new THREE.Mesh(pinGeometry, pinMaterial);
            in2.rotation.x = Math.PI / 2;
            in2.position.set(-0.5, -0.2, 0);
            in2.userData = { isPin: true, direction: 'input', name: 'IN2' };
            group.add(in2);
        }
        
        // Add output pin (right side)
        if (type !== 'SWITCH') {
            const out = new THREE.Mesh(pinGeometry, pinMaterial);
            out.rotation.x = Math.PI / 2;
            out.position.set(0.5, 0, 0);
            out.userData = { isPin: true, direction: 'output', name: 'OUT' };
            group.add(out);
        }
    }
    
    addLabel(group, text) {
        // Simple label using canvas texture
        const canvas = document.createElement('canvas');
        canvas.width = 256;
        canvas.height = 64;
        const ctx = canvas.getContext('2d');
        ctx.fillStyle = '#ffffff';
        ctx.font = 'Bold 32px Arial';
        ctx.textAlign = 'center';
        ctx.fillText(text, 128, 40);
        
        const texture = new THREE.CanvasTexture(canvas);
        const material = new THREE.SpriteMaterial({ map: texture });
        const sprite = new THREE.Sprite(material);
        sprite.position.set(0, 1, 0);
        sprite.scale.set(1.5, 0.5, 1);
        group.add(sprite);
    }
    
    createWire(comp1, comp2) {
        const points = [];
        const start = comp1.position.clone();
        const end = comp2.position.clone();
        
        // Add arc control point
        const mid = start.clone().lerp(end, 0.5);
        mid.y += 0.5;
        
        // Create curve
        const curve = new THREE.QuadraticBezierCurve3(start, mid, end);
        const geometry = new THREE.TubeGeometry(curve, 20, 0.03, 8, false);
        const material = new THREE.MeshPhongMaterial({ 
            color: 0x666666,
            emissive: 0x222222
        });
        
        const wire = new THREE.Mesh(geometry, material);
        wire.userData = {
            isWire: true,
            from: comp1,
            to: comp2,
            signal: false
        };
        
        this.scene.add(wire);
        this.wires.push(wire);
        
        this.updateStatistics();
    }
    
    updateCameraPosition() {
        const x = this.cameraTarget.x + this.cameraDistance * Math.sin(this.cameraAngle.x) * Math.cos(this.cameraAngle.y);
        const y = this.cameraTarget.y + this.cameraDistance * Math.sin(this.cameraAngle.y);
        const z = this.cameraTarget.z + this.cameraDistance * Math.cos(this.cameraAngle.x) * Math.cos(this.cameraAngle.y);
        
        this.camera.position.set(x, y, z);
        this.camera.lookAt(this.cameraTarget.x, this.cameraTarget.y, this.cameraTarget.z);
    }
    
    setupEventListeners() {
        // Window resize
        window.addEventListener('resize', () => this.resizeCanvas());
        
        // Mouse controls
        let isDragging = false;
        let previousMousePosition = { x: 0, y: 0 };
        
        this.canvas.addEventListener('mousedown', (e) => {
            isDragging = true;
        });
        
        this.canvas.addEventListener('mousemove', (e) => {
            if (isDragging) {
                const deltaX = e.clientX - previousMousePosition.x;
                const deltaY = e.clientY - previousMousePosition.y;
                
                this.cameraAngle.x += deltaX * 0.01;
                this.cameraAngle.y += deltaY * 0.01;
                
                // Clamp vertical angle
                this.cameraAngle.y = Math.max(-Math.PI/2 + 0.1, Math.min(Math.PI/2 - 0.1, this.cameraAngle.y));
                
                this.updateCameraPosition();
            }
            
            previousMousePosition = { x: e.clientX, y: e.clientY };
        });
        
        this.canvas.addEventListener('mouseup', () => {
            isDragging = false;
        });
        
        this.canvas.addEventListener('wheel', (e) => {
            this.cameraDistance += e.deltaY * 0.01;
            this.cameraDistance = Math.max(2, Math.min(50, this.cameraDistance));
            this.updateCameraPosition();
        });
        
        // Keyboard controls
        document.addEventListener('keydown', (e) => {
            switch(e.key.toLowerCase()) {
                case 'r':
                    this.resetCamera();
                    break;
                case ' ':
                    this.togglePause();
                    break;
                case 'escape':
                    this.deselectAll();
                    break;
            }
        });
        
        // Tool buttons
        document.querySelectorAll('.tool-btn').forEach(btn => {
            btn.addEventListener('click', () => {
                document.querySelectorAll('.tool-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                this.currentTool = btn.dataset.tool;
                this.updateStatusMessage(`Tool: ${btn.textContent}`);
            });
        });
        
        // Component palette
        document.querySelectorAll('.palette-item').forEach(item => {
            item.addEventListener('click', () => {
                const type = item.dataset.type;
                this.spawnComponent(type);
                this.updateStatusMessage(`Selected: ${type} component`);
            });
        });
    }
    
    spawnComponent(type) {
        const position = {
            x: (Math.random() - 0.5) * 4,
            y: 0.5,
            z: (Math.random() - 0.5) * 2
        };
        
        const colors = {
            'AND': 0x2ecc71,
            'OR': 0x3498db,
            'NOT': 0xe74c3c,
            'XOR': 0xf1c40f,
            'SWITCH': 0x95a5a6,
            'LED': 0xe67e22,
            'MONITOR': 0x9b59b6,
            'CLOCK': 0x1abc9c,
            'CHIP': 0x34495e
        };
        
        const component = this.createComponent(type.toUpperCase(), position, colors[type.toUpperCase()] || 0x888888);
        this.components.push(component);
        this.updateStatistics();
    }
    
    resetCamera() {
        this.cameraAngle = { x: 0.5, y: 0.5 };
        this.cameraDistance = 10;
        this.cameraTarget = { x: 0, y: 0, z: 0 };
        this.updateCameraPosition();
    }
    
    togglePause() {
        this.isPaused = !this.isPaused;
        document.getElementById('stat-sim').textContent = this.isPaused ? 'Paused' : 'Running';
        this.updateStatusMessage(this.isPaused ? 'Simulation paused' : 'Simulation resumed');
    }
    
    deselectAll() {
        if (this.selectedComponent) {
            this.selectedComponent.children.forEach(child => {
                if (child.material) {
                    child.material.emissive.setHex(0x000000);
                }
            });
            this.selectedComponent = null;
        }
    }
    
    resizeCanvas() {
        this.canvas.width = window.innerWidth;
        this.canvas.height = window.innerHeight;
        
        if (this.renderer) {
            this.renderer.setSize(this.canvas.width, this.canvas.height);
        }
        
        if (this.camera) {
            this.camera.aspect = this.canvas.width / this.canvas.height;
            this.camera.updateProjectionMatrix();
        }
    }
    
    updateStatistics() {
        document.getElementById('stat-components').textContent = this.components.length;
        document.getElementById('stat-wires').textContent = this.wires.length;
        document.getElementById('stat-chips').textContent = this.components.filter(c => 
            c.userData.type === 'CHIP').length;
    }
    
    updateStatusMessage(message) {
        document.getElementById('status-message').textContent = message;
    }
    
    animate(currentTime = 0) {
        requestAnimationFrame((t) => this.animate(t));
        
        // Calculate FPS
        const deltaTime = currentTime - this.lastFrameTime;
        this.lastFrameTime = currentTime;
        
        if (deltaTime > 0) {
            this.fps = Math.round(1000 / deltaTime);
            document.getElementById('stat-fps').textContent = this.fps;
        }
        
        if (!this.isPaused) {
            // Animate components
            this.components.forEach(comp => {
                // Gentle floating animation
                comp.position.y += Math.sin(currentTime * 0.001 + comp.position.x) * 0.001;
            });
            
            // Animate wires
            this.wires.forEach(wire => {
                if (wire.userData.signal) {
                    wire.material.emissive.setHex(0x4fc3f7);
                } else {
                    wire.material.emissive.setHex(0x222222);
                }
            });
        }
        
        this.renderer.render(this.scene, this.camera);
    }
}

// Initialize game when Three.js is loaded
function initGame() {
    window.game = new ElectronicSandboxGame();
}

// Load Three.js from CDN
const script = document.createElement('script');
script.src = 'https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js';
script.onload = initGame;
document.head.appendChild(script);
