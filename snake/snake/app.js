// ===================== 贪吃蛇游戏配置 =====================
const WIDTH = 40; // 游戏区域格子总宽度（单位：格子）
const HEIGHT = 40; // 游戏区域格子总高度（单位：格子）
const BLOCK_SIZE = 16; // 每个格子像素大小
const canvas = document.getElementById('gameCanvas'); // 获取画布DOM元素
const ctx = canvas.getContext('2d'); // 获取2D绘图上下文
const scoreText = document.getElementById('score'); // 分数显示DOM
const timeText = document.getElementById('time'); // 存活时间显示DOM
const MOVE_INTERVAL = 120; // 蛇移动间隔(毫秒)，数值越小速度越快
let lastTime = 0; // 上一次执行游戏逻辑的时间戳，用于控速
let startTime = 0; // 游戏真正开始的时间戳（第一次按键才启动计时）
let isPlaying = false; // 是否处于游戏进行中状态
let surviveSecond = 0; // 保存本局存活总秒数
// ===================== Web Audio 复古蜂鸣音效模块 =====================
let audioCtx = null; // 音频上下文对象，浏览器要求用户交互后才能初始化
/**
 * 初始化音频上下文
 * 浏览器安全策略：不能页面加载直接创建AudioContext，必须在点击/按键等用户交互后创建
 */
function initAudio() {
    if (!audioCtx) {
        audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    }
}
/**
 * 播放蜂鸣音效
 * @param {number} freq 声音频率(Hz)
 * @param {number} duration 持续时间(ms)
 */
function beep(freq, duration) {
    initAudio();
    const osc = audioCtx.createOscillator(); // 创建振荡器，生成声波
    const gain = audioCtx.createGain(); // 音量增益控制器
    osc.type = "square"; // 方波，复古街机音色
    osc.frequency.value = freq; // 设置振动频率
    gain.gain.value = 0.07; // 音量，不要太大避免刺耳
    osc.connect(gain); // 振荡器输出连接到音量控制器
    gain.connect(audioCtx.destination); // 最终输出到扬声器
    osc.start();
    setTimeout(() => osc.stop(), duration); // 定时停止发声
}
// 吃到食物音效
function soundEat() {
    beep(680, 70);
}
// 死亡音效
function soundDie() {
    beep(160, 300);
}
// 重启音效
function soundReset() {
    beep(450, 90);
}
// 设置画布实际像素宽高 = 格子数 × 单格像素
canvas.width = WIDTH * BLOCK_SIZE;
canvas.height = HEIGHT * BLOCK_SIZE;

// ===================== 方向枚举常量 =====================
const Dir = {
    STOP: 0,
    LEFT: 1,
    RIGHT: 2,
    UP: 3,
    DOWN: 4
};

let dir = Dir.STOP; // 当前蛇移动方向
let snakeX, snakeY; // 蛇头格子坐标
let foodX, foodY; // 食物格子坐标
let tailX = []; // 蛇身体每一节X坐标数组
let tailY = []; // 蛇身体每一节Y坐标数组
let nTail = 0; // 蛇身体节数（不包含蛇头）
let gameOver = false; // 游戏结束标记

// 初始化/重启游戏
function Setup() {
    gameOver = false;
    dir = Dir.STOP; // 蛇静止，等待玩家按键才开始移动
    // 蛇出生在中心
    snakeX = Math.floor(WIDTH / 2);
    snakeY = Math.floor(HEIGHT / 2);
    nTail = 0; // 身体长度清零
    tailX = [];
    tailY = [];
    isPlaying = false; // 游戏未开始，等待第一次方向按键
    startTime = 0; // 清空开始计时时间戳
    SpawnFood(); // 生成第一颗食物
    UpdateScore(); // 更新分数UI
}

// 生成食物，保证不刷在蛇身上
function SpawnFood() {
    let ok;
    do {
        ok = true;
        // 1 ~ WIDTH‑2，避开四周围墙
        foodX = Math.floor(Math.random() * (WIDTH - 2)) + 1;
        foodY = Math.floor(Math.random() * (HEIGHT - 2)) + 1;
        // 判断是否和蛇重合
        if (snakeX === foodX && snakeY === foodY) ok = false;
        for (let i = 0; i < nTail; i++) {
            if (tailX[i] === foodX && tailY[i] === foodY) {
                ok = false;
                break;
            }
        }
    } while (!ok); // 如果位置冲突，循环重新生成
}
/**
 * UpdateScore：更新页面分数文本
 * 身体每一节100分
 */
function UpdateScore() {
    scoreText.innerText = nTail * 100;
}

/**
 * Draw：渲染一帧画面
 * @param {number} timestamp requestAnimationFrame传入的时间戳，用于动画
 */
function Draw(timestamp) {
    // 1.填充画布背景底色
    ctx.fillStyle = "#c5d0c0";
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // 绘制淡网格辅助线
    ctx.strokeStyle = "#b2bcad";
    ctx.lineWidth = 0.4;
    for (let x = 0; x <= WIDTH; x++) {
        ctx.beginPath();
        ctx.moveTo(x * BLOCK_SIZE, 0);
        ctx.lineTo(x * BLOCK_SIZE, canvas.height);
        ctx.stroke();
    }
    for (let y = 0; y <= HEIGHT; y++) {
        ctx.beginPath();
        ctx.moveTo(0, y * BLOCK_SIZE);
        ctx.lineTo(canvas.width, y * BLOCK_SIZE);
        ctx.stroke();
    }

    // 绘制四周围墙，使用线性渐变增加立体感
    const wallGrad = ctx.createLinearGradient(0, 0, 0, BLOCK_SIZE);
    wallGrad.addColorStop(0, '#888888');
    wallGrad.addColorStop(1, '#444444');
    ctx.fillStyle = wallGrad;
    // 上下围墙
    for (let x = 0; x < WIDTH; x++) {
        DrawBlock(x, 0);
        DrawBlock(x, HEIGHT - 1);
    }
    // 左右围墙
    for (let y = 0; y < HEIGHT; y++) {
        DrawBlock(0, y);
        DrawBlock(WIDTH - 1, y);
    }

    // 食物呼吸缩放动画，利用正弦函数实现大小周期性变化
    const pulse = Math.sin(timestamp / 180) * 2;

    // 绘制食物主体红色方块
    ctx.fillStyle = "#ff3333";
    ctx.fillRect(
        foodX * BLOCK_SIZE + pulse,
        foodY * BLOCK_SIZE + pulse,
        BLOCK_SIZE - 1 - pulse * 2,
        BLOCK_SIZE - 1 - pulse * 2
    );
    // 食物高光小块，模拟反光效果
    ctx.fillStyle = "rgba(255,255,255,0.35)";
    ctx.fillRect(
        foodX * BLOCK_SIZE + 3 + pulse,
        foodY * BLOCK_SIZE + 3 + pulse,
        5,
        5
    );

    // 绘制蛇身体
    ctx.fillStyle = "#2fc948";
    for (let i = 0; i < nTail; i++) {
        DrawBlock(tailX[i], tailY[i]);
    }

    // 绘制蛇头
    ctx.fillStyle = "#4affdd";
    DrawBlock(snakeX, snakeY);

    // 蛇头外发光效果
    ctx.shadowColor = "#4affdd";
    ctx.shadowBlur = 8;
    ctx.fillRect(
        snakeX * BLOCK_SIZE + 2,
        snakeY * BLOCK_SIZE + 2,
        BLOCK_SIZE - 5,
        BLOCK_SIZE - 5
    );
    ctx.shadowBlur = 0; // 关闭发光，避免影响其他绘制

    // 游戏结束遮罩与文字提示
    if (gameOver) {
        ctx.fillStyle = "rgba(0,0,0,0.6)"; // 半透明黑色遮罩
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        ctx.fillStyle = "#ff2222";
        ctx.font = "30px Microsoft Yahei";
        ctx.textAlign = "center";
        ctx.fillText("游戏结束！按 R 重新开始", canvas.width / 2, canvas.height / 2);
    }
}

/**
 * DrawBlock：绘制单个格子方块
 * @param {number} x 格子横坐标(格子单位，不是像素)
 * @param {number} y 格子纵坐标(格子单位，不是像素)
 */
function DrawBlock(x, y) {
    ctx.fillRect(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE - 1, BLOCK_SIZE - 1);
}

/**
 * Logic：游戏核心逻辑更新
 * 每 MOVE_INTERVAL 毫秒执行一次，控制蛇移动、碰撞判定、吃食物
 */
function Logic() {
    if (gameOver) return; // 游戏结束不再执行逻辑

    /// ========== 蛇身体跟随算法 ==========
    // 保存旧坐标，把每一节身体向前挪，后一节跟上前一节的位置
    let prevX = tailX[0];
    let prevY = tailY[0];
    let prev2X, prev2Y;
    tailX[0] = snakeX;
    tailY[0] = snakeY;

    for (let i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    // 根据方向更新蛇头坐标
    switch (dir) {
        case Dir.LEFT: snakeX--; break;
        case Dir.RIGHT: snakeX++; break;
        case Dir.UP: snakeY--; break;
        case Dir.DOWN: snakeY++; break;
    }

    // ========== 撞墙判定 ==========
    // 0和WIDTH‑1/HEIGHT‑1是围墙，碰到就死亡
    if (snakeX <= 0 || snakeX >= WIDTH - 1 || snakeY <= 0 || snakeY >= HEIGHT - 1) {
        gameOver = true;
        isPlaying = false;
        soundDie(); // 死亡音效
    }

    // ========== 撞到自身身体判定 ==========
    for (let i = 0; i < nTail; i++) {
        if (tailX[i] === snakeX && tailY[i] === snakeY) {
            gameOver = true;
            isPlaying = false;
            soundDie(); // 死亡音效
        }
    }

    // ========== 吃到食物 ==========
    if (snakeX === foodX && snakeY === foodY) {
        nTail++;
        UpdateScore();
        SpawnFood();
        soundEat(); // 吃食物音效
        // 可在此处写加速逻辑：MOVE_INTERVAL -= 5;
    }
}

// 键盘监听
document.addEventListener("keydown", (e) => {
    // R键：重启游戏，游戏结束/游玩中都可以触发
    if (e.key.toLowerCase() === 'r') {
        Setup();
        soundReset(); // 重启音效
        return;
    }
    if (gameOver) return;

    if (!isPlaying) {
        isPlaying = true;
        startTime = performance.now(); // 真正按下按键这一刻才开启计时
    }
    // 方向控制：禁止直接向反方向掉头（向右不能直接向左）
    switch (e.key) {
        case "a": case "A": case "ArrowLeft":
            if (dir !== Dir.RIGHT) dir = Dir.LEFT;
            break;
        case "d": case "D": case "ArrowRight":
            if (dir !== Dir.LEFT) dir = Dir.RIGHT;
            break;
        case "w": case "W": case "ArrowUp":
            if (dir !== Dir.DOWN) dir = Dir.UP;
            break;
        case "s": case "S": case "ArrowDown":
            if (dir !== Dir.UP) dir = Dir.DOWN;
            break;
    }
});

/**
 * GameLoop：主游戏循环
 * requestAnimationFrame驱动，约60帧每秒；使用时间戳做时间控速，不依赖帧率
 * @param {number} timestamp 浏览器传入高精度时间戳
 */
function GameLoop(timestamp) {
    requestAnimationFrame(GameLoop); // 递归调用，持续渲染下一帧
    const delta = timestamp - lastTime; // 间隔到达才执行蛇移动逻辑，和页面帧率解耦
    if (delta >= MOVE_INTERVAL) {
        Logic();
        lastTime = timestamp;
    }
    // 更新存活时间UI
    if (isPlaying) {
        const playSec = Math.floor((timestamp - startTime) / 1000);
        timeText.innerText = playSec;
        surviveSecond = playSec; // 实时记录当前时长
    } else {
        timeText.innerText = surviveSecond; // 游戏暂停或结束时显示最后存活时长
    } 
    Draw(timestamp); // 每帧都绘制画面
}

Setup();
// 帧率控制，对应C++ Sleep()，这里用requestAnimationFrame实现
requestAnimationFrame(GameLoop);