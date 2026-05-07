#include "app/dsl_app.h"

#include "components/components.h"
#include "diary/encrypt.h"
#include "diary/diary_data.h"

#include <algorithm>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>

namespace app {

static std::string g_dailyQuote;
static bool g_quoteInitialized = false;

static std::atomic<bool> g_loginResultReady{false};
static std::atomic<bool> g_loginSuccess{false};

static const char* kDailyQuotes[] = {
    "人生就像一杯茶，不会苦一辈子，但总会苦一阵子。",
    "所有的伟大，都源于一个勇敢的开始。",
    "人生没有白走的路，每一步都算数。",
    "温柔半两，从容一生。",
    "往事不回头，余生不将就。",
    "心有猛虎，细嗅蔷薇。",
    "人生如逆旅，我亦是行人。",
    "浮生若梦，为欢几何。",
    "人生如棋，落子无悔。",
    "岁月静好，现世安稳。",
    "往事随风，余生不将就。",
    "心有山海，静而无边。",
    "人生海海，山山而川。",
    "时光不语，静待花开。",
    "落花时节，又逢君。",
    "人间烟火气，最抚凡人心。",
    "偷得浮生半日闲。",
    "当时只道是寻常。",
    "一蓑烟雨任平生。",
    "世事一场大梦，人生几度秋凉。",
    "人生若只如初见。",
    "何事秋风悲画扇。",
    "曾经沧海难为水。",
    "除却巫山不是云。",
    "两情若是久长时，又岂在朝朝暮暮。",
    "人生得意须尽欢，莫使金樽空对月。",
    "天生我材必有用，千金散尽还复来。",
    "采菊东篱下，悠然见南山。",
    "问君何能尔，心远地自偏。",
    "山气日夕佳，飞鸟相与还。",
    "此中有真意，欲辨已忘言。",
    "人生如梦，一尊还酹江月。",
    "大江东去，浪淘尽，千古风流人物。",
    "故垒西边，人道是，三国周郎赤壁。",
    "人生到处知何似，应似飞鸿踏雪泥。",
    "泥上偶然留指爪，鸿飞那复计东西。",
    "但愿人长久，千里共婵娟。",
    "人有悲欢离合，月有阴晴圆缺。",
    "此事古难全，但愿人长久。",
    "人生如寄，多忧何为。",
    "今朝有酒今朝醉，明日愁来明日愁。",
    "抽刀断水水更流，举杯消愁愁更愁。",
    "人生在世不称意，明朝散发弄扁舟。",
    "两岸猿声啼不住，轻舟已过万重山。",
    "沉舟侧畔千帆过，病树前头万木春。",
    "山重水复疑无路，柳暗花明又一村。",
    "世事洞明皆学问，人情练达即文章。",
    "假作真时真亦假，无为有处有还无。",
    "身后有馀忘缩手，眼前无路想回头。",
    "纵浪大化中，不喜亦不惧。",
    "简单的生活，简单的幸福。",
    "少即是多，慢即是快。",
    "日子平淡，好在我喜欢。",
    "人间值得，未来可期。",
    "慢慢来，比较快。",
    "温柔自有力量。",
    "生活明朗，万物可爱。",
    "心有归处，身有依靠。",
    "平安喜乐，顺遂无忧。",
    "日子有盼头，生活有奔头。",
    "把寻常的日子过成诗。",
    "生活很苦，但你很甜。",
    "眼里有光，心里有爱。",
    "活得热烈，过得自在。",
    "简单点，开心点。",
    "日子在发芽，生活在开花。",
    "生活的理想，就是为了理想的生活。",
    "愿你眼里全是星光，笑里全是坦荡。",
    "温柔是一种选择，快乐是一种能力。",
    "把柴米油盐过成诗酒花茶。",
    "人间烟火气，最抚凡人心。",
    "日子渺小重复，却都是幸福。",
    "好好生活，慢慢相遇。",
    "生活需要仪式感，但不需要表演。",
    "愿你每一天都奔赴在自己的热爱里。",
    "平凡的日子也泛着光。",
    "认真生活，才能找到被藏起来的糖。",
    "日子是自己的，要过得有滋味。",
    "生活再难，也要记得微笑。",
    "愿你心之所向，皆是美好。",
    "把平凡的日子过成喜欢的样子。",
    "日子慢慢来，美好慢慢品。",
    "生活不是选择，而是热爱。",
    "愿你一生努力，一生被爱。",
    "好好生活，珍惜当下。",
    "日子有趣有盼，不负心中热爱。",
    "生活的本质，是平淡。",
    "愿你忠于自己，活得认真。",
    "日子简单过，心事放下说。",
    "生活不是为了赶路，而是为了感受路。",
    "愿你眼里有星河，笑里有清风。",
    "把每一个平凡的日子，过成诗。",
    "日子再忙，也要记得吃饭。",
    "生活的温柔，藏在每一个清晨。",
    "愿你一生清澈明朗。",
    "日子简单，人间值得。",
    "好好生活，慢慢变好。",
    "生活最好的状态，是未来可期。",
    "愿你所求皆如愿，所行皆坦途。",
    "把日子过成自己想要的样子。",
    "有些事，经历了才懂得。",
    "有些路，走过了才明白。",
    "有些人错过了就是一辈子。",
    "所有的相遇，都有意义。",
    "陪伴是最长情的告白。",
    "愿你有人问你粥可温，有人与你立黄昏。",
    "时光不老，我们不散。",
    "聚散离合，都是风景。",
    "有些人光是遇见就很幸运了。",
    "愿你历尽千帆，归来仍是少年。",
    "所有的离别，都是另一种开始。",
    "珍惜所有的不期而遇。",
    "看淡所有的渐行渐远。",
    "有些人就是用来错过的。",
    "相遇是缘，相知是份。",
    "愿你遇到一个懂你的人。",
    "最好的关系，是相处不累。",
    "愿你有人陪伴，不再孤单。",
    "有些人出现，是为了照亮你。",
    "所有的离开，都在等待重逢。",
    "愿你的身边，永远有人陪伴。",
    "有些话，说出来就是伤害。",
    "愿你被世界温柔以待。",
    "最好的时光，是有你在身边。",
    "愿你的眼泪都是喜极而泣。",
    "有些人注定是过客。",
    "愿你遇到的都是真心。",
    "所有的思念，都是因为在乎。",
    "愿你余生有人陪，再不孤单。",
    "有些遗憾，也是一种美。",
    "愿你的故事，都有圆满的结局。",
    "所有的等待，都是为了更好的相遇。",
    "愿你遇到一个愿意听你说话的人。",
    "有些人出现，就是来教会你什么。",
    "愿你余生有人懂你的欲言又止。",
    "所有的磨合，都是为了更靠近。",
    "愿你身边永远有一个懂你的人。",
    "有些相遇，是命中注定。",
    "愿你所有的坚持，都是因为热爱。",
    "所有的改变，都是因为成长。",
    "愿你一生被爱，一生可爱。",
    "有些路，注定要一个人走。",
    "愿你学会独处，也享受热闹。",
    "所有的孤独，都是为了等待。",
    "愿你遇见一个能让你做自己的人。",
    "有些事情，放下才会轻松。",
    "愿你学会放下，学会原谅。",
    "所有的经历，都是财富。",
    "愿你活得通透，看得明白。",
    "有些道理，需要时间来懂。",
    "做一个温暖的人，浅浅笑轻轻爱。",
    "愿你成为自己的太阳，无需凭借谁的光。",
    "你若盛开，清风自来。",
    "愿你温柔且坚定，知足且上进。",
    "保持热爱，奔赴山海。",
    "愿你心有猛虎，也能细嗅蔷薇。",
    "做自己的英雄，也做自己的小孩。",
    "愿你活得任性，也能收放自如。",
    "温柔要有，但不是妥协。",
    "愿你既有诗和远方，也有烟火日常。",
    "把自己活成一道光。",
    "愿你所有的坚持，都是因为值得。",
    "做一个明媚的女子，不倾国不倾城，以颜悦人。",
    "愿你永远年轻，永远热泪盈眶。",
    "愿你出走半生，归来仍是少年。",
    "保持纯真，保持善良。",
    "愿你眼里有星辰，心中有山海。",
    "做一个温暖而有力量的人。",
    "愿你历尽沧桑，依然相信美好。",
    "保持初心，方得始终。",
    "愿你永远善良，永远热忱。",
    "做一个温柔的人，温暖自己，温暖他人。",
    "愿你被这个世界温柔以待，也能温柔待人。",
    "保持热爱，是生活最好的姿态。",
    "愿你成为自己喜欢的样子。",
    "不讨好，不强求，不将就。",
    "愿你所有的努力，都有回报。",
    "做一个内心强大的人。",
    "愿你学会坚强，也学会柔软。",
    "保持真实，保持坦荡。",
    "愿你活得通透，活得明白。",
    "做自己想做的事，成为自己想成为的人。",
    "愿你不负韶华，不负自己。",
    "保持一颗平常心。",
    "愿你所有的等待，都值得。",
    "做一个自信的人，不卑不亢。",
    "愿你永远相信美好的事情正在发生。",
    "保持热情，保持好奇。",
    "愿你永远不失勇气，永远热泪盈眶。",
    "做一个简单而纯粹的人。",
    "愿你所有的坚持，都源于热爱。",
    "保持清醒，保持温柔。",
    "愿你成为自己的光，照亮自己的路。",
    "做一个内心有光的人。",
    "愿你永远善良，永远相信。",
    "保持对生活的热爱。",
    "愿你活得自由，活得坦荡。",
    "做自己的摆渡人。",
    "愿你所有的苦难，都是成长的铺垫。",
    "保持热爱，奔赴远方。",
    "心若不动，风又奈何。",
    "你若不伤，岁月无恙。",
    "随心随缘，随遇而安。",
    "淡泊明志，宁静致远。",
    "往事清零，爱恨随意。",
    "所得皆所期，所失皆无碍。",
    "不负时光，不负自己。",
    "心有山海，静而无边。",
    "往事不回头，余生不将就。",
    "温柔半两，从容一生。",
    "心有归处，身有依靠。",
    "平安喜乐，顺遂无忧。",
    "日子有盼头，生活有奔头。",
    "愿你眼里全是星光。",
    "愿你笑里全是坦荡。",
    "温柔是一种选择。",
    "把日子过成诗。",
    "眼里有光，心中有爱。",
    "日子慢慢来，美好慢慢品。",
    "生活不是选择，而是热爱。",
    "人间值得，未来可期。",
    "把寻常的日子过成诗。",
    "简单点，开心点。",
    "活在当下，珍惜眼前。",
    "愿你历尽千帆，归来仍是少年。",
    "心若向阳，无畏悲伤。",
    "以梦为马，不负韶华。",
    "不忘初心，方得始终。",
    "保持热爱，奔赴山海。",
    "时间是最好的良药。",
    "岁月从不败美人。",
    "愿你此生尽兴，赤诚善良。",
    "愿你所有快乐，无需假装。",
    "愿你此去经年，初心不改。",
    "愿你历尽山河，觉得人间值得。",
    "愿你远行星辰大海，归来仍是少年。",
    "愿你走过半生，归来仍是少年。",
    "愿你眼中有星辰，心中有山海。",
    "愿你所到之处，皆是风景。",
    "愿你遇见良人，不负余生。",
    "愿你所有坚持，都是因为热爱。",
    "愿你所有等待，都有答案。",
    "愿你所有深情，都不被辜负。",
    "愿你所有努力，都能开花结果。",
    "愿你所有坚持，都能照亮前路。",
    "愿你所有温柔，都能被人珍视。",
    "愿你所有善良，都能被人看见。",
    "愿你所有付出，都能被人珍惜。",
    "愿你所有梦想，都能成真。",
    "时光清浅，岁月静好。",
    "愿你被时光温柔以待。",
    "愿你走过平湖烟雨，岁月山河。",
    "愿你历经世事，眼里仍是星辰。",
    "愿你看透人间，依然热爱生活。",
    "愿你走出半生，归来仍是少年。",
    "愿你历尽千帆，心中仍有火焰。",
    "愿你穿过风雨，依然阳光明媚。",
    "愿你踏遍山河，觉得人间值得。",
    "愿你遍历人间，觉得岁月静好。",
    "愿你看尽繁华，觉得简单最美。",
    "愿你尝尽百味，觉得平淡是真。",
    "愿你走遍天涯，觉得家是最暖。",
    "愿你历尽沧桑，觉得初心最珍贵。",
    "愿你看过万千风景，觉得当下最好。",
    "愿你经过悲欢离合，觉得团圆最暖。",
    "愿你经历风风雨雨，觉得晴天最美。",
    "愿你尝遍酸甜苦辣，觉得甜蜜最真。",
    "愿你看过日出日落，觉得陪伴最长情。",
    "愿你经过四季轮回，觉得春天最温柔。",
    "愿你走过白天黑夜，觉得星光最浪漫。",
    "愿你看过云卷云舒，觉得自在最难得。",
    "愿你经过潮起潮落，觉得平静最珍贵。",
    "愿你看过花开花落，觉得绽放最美丽。",
    "愿你经过月圆月缺，觉得团圆最重要。",
    "愿你看过风起风停，觉得平稳最幸福。",
    "愿你经过春夏秋冬，觉得每一天都值得。",
    "愿你看过日出日落，觉得每一天都很美。",
    "愿你走过千山万水，觉得家是最好的归处。",
    "愿你历经千帆，觉得人间值得。"
};

enum class Page {
    FirstLaunchSetPassword,
    Login,
    MainMenu
};

enum class MainMenuSelection {
    None = -1,
    New = 0,
    Edit = 1,
    Delete = 2,
    List = 3,
    Settings = 4
};

enum class ThemeSetting {
    Dark = 0,
    Light = 1,
};

enum class BackgroundEffect {
    Acrylic = 0,
    FrostedGlass = 1,
};

struct AppState {
    Page currentPage = Page::Login;
    MainMenuSelection mainMenuSelection = MainMenuSelection::None;
    std::string password;
    std::string toastTitle;
    std::string toastMessage;
    bool showToast = false;
    bool welcomeShown = false;

    std::string diaryContent;
    std::string userName;

    std::vector<diary::DiaryEntry> diaryList;
    int selectedDiaryIndex = -1;

    bool showDeleteConfirm = false;
    bool deleteConfirmed = false;

    bool initialized = false;

    ThemeSetting themeSetting = ThemeSetting::Light;
    BackgroundEffect backgroundEffect = BackgroundEffect::Acrylic;

    float listScrollOffset = 0.0f;
    float editListScrollOffset = 0.0f;
    float deleteListScrollOffset = 0.0f;

    bool loginVerifying = false;
    float loginProgress = 0.0f;
    std::chrono::time_point<std::chrono::steady_clock> loginStartTime;
    std::shared_ptr<std::thread> loginThread;
};

static AppState& state() {
    static AppState s;
    return s;
}

static components::theme::ThemeColorTokens currentThemeColors() {
    AppState& s = state();
    if (s.themeSetting == ThemeSetting::Light) {
        return components::theme::light();
    }
    return components::theme::dark();
}

static std::pair<core::Color, core::Color> getPanelGradient(const components::theme::ThemeColorTokens& colors) {
    AppState& s = state();
    if (s.backgroundEffect == BackgroundEffect::FrostedGlass) {
        if (colors.dark) {
            return {
                components::theme::withAlpha(colors.surface, 0.75f),
                components::theme::withAlpha(colors.surface, 0.65f)
            };
        } else {
            return {
                components::theme::withAlpha(colors.surface, 0.85f),
                components::theme::withAlpha(colors.surface, 0.75f)
            };
        }
    }
    // Acrylic effect
    if (colors.dark) {
        return {
            components::theme::color(0.10f, 0.12f, 0.16f, 1.0f),
            components::theme::color(0.05f, 0.07f, 0.10f, 1.0f)
        };
    } else {
        return {
            components::theme::color(1.00f, 1.00f, 1.00f, 1.0f),
            components::theme::color(0.95f, 0.95f, 0.97f, 1.0f)
        };
    }
}

static core::Color getPanelBorder(const components::theme::ThemeColorTokens& colors) {
    if (colors.dark) {
        return components::theme::color(0.23f, 0.29f, 0.38f, 1.0f);
    } else {
        return components::theme::color(0.65f, 0.70f, 0.75f, 1.0f);
    }
}

static void showToastMessage(const std::string& title, const std::string& message) {
    AppState& s = state();
    s.toastTitle = title;
    s.toastMessage = message;
    s.showToast = true;
    printf("[DEBUG] Toast shown: %s - %s\n", title.c_str(), message.c_str());
}

static void goToPage(Page page) {
    AppState& s = state();
    s.currentPage = page;
    if (page == Page::MainMenu) {
        s.mainMenuSelection = MainMenuSelection::None;
    }
    detail::dslRuntime().markFullRedraw();
}

static std::string getDailyQuote() {
    if (!g_quoteInitialized) {
        srand(static_cast<unsigned>(time(nullptr)));
        g_quoteInitialized = true;
    }
    const size_t quoteCount = sizeof(kDailyQuotes) / sizeof(kDailyQuotes[0]);
    size_t index = static_cast<size_t>(rand()) % quoteCount;
    return kDailyQuotes[index];
}

const DslAppConfig& dslAppConfig() {
    if (g_dailyQuote.empty()) {
        g_dailyQuote = getDailyQuote();
    }
    static std::string fullTitle = std::string("安全日记本 丨 ") + g_dailyQuote;
    static const DslAppConfig config = DslAppConfig{}
        .title(fullTitle.c_str())
        .pageId("diary")
        .clearColor({0.12f, 0.14f, 0.16f, 1.0f})
        .windowSize(1000, 700)
        .iconPath("assets/icon.png");
    return config;
}

static void composeFirstLaunchSetPassword(core::dsl::Ui& ui, const core::dsl::Screen& screen) {
    AppState& s = state();
    auto colors = currentThemeColors();
    auto panelGrad = getPanelGradient(colors);
    auto panelBorder = getPanelBorder(colors);

    ui.stack("first_launch")
        .size(screen.width, screen.height)
        .align(core::Align::CENTER, core::Align::CENTER)
        .content([&] {
            components::panel(ui, "fl_card")
                .size(420.0f, 400.0f)
                .radius(18.0f)
                .gradient(panelGrad.first, panelGrad.second)
                .border(1.0f, panelBorder)
                .shadow(26.0f, 0.0f, 8.0f, {0.0f, 0.0f, 0.0f, 0.26f})
                .build();

            ui.column("fl_content")
                .size(420.0f, 400.0f)
                .gap(8.0f)
                .justifyContent(core::Align::CENTER)
                .alignItems(core::Align::CENTER)
                .content([&] {
                    components::text(ui, "fl_title")
                        .size(360.0f, 36.0f)
                        .text("欢迎首次使用安全日记本")
                        .fontSize(22.0f)
                        .lineHeight(28.0f)
                        .color(components::theme::withAlpha(colors.text, 0.98f))
                        .horizontalAlign(core::HorizontalAlign::Center)
                        .build();

                    components::text(ui, "fl_name_label")
                        .size(360.0f, 24.0f)
                        .text("该怎么称呼您？")
                        .fontSize(16.0f)
                        .lineHeight(22.0f)
                        .color(components::theme::withAlpha(colors.text, 0.72f))
                        .horizontalAlign(core::HorizontalAlign::Center)
                        .build();

                    components::input(ui, "fl_name_input")
                        .size(320.0f, 48.0f)
                        .text(s.userName)
                        .placeholder("请输入您的名字")
                        .fontSize(16.0f)
                        .inset(14.0f)
                        .theme(colors)
                        .onChange([&](const std::string& value) {
                            s.userName = value;
                        })
                        .build();

                    components::text(ui, "fl_password_label")
                        .size(360.0f, 24.0f)
                        .text("请设置您的主密码")
                        .fontSize(16.0f)
                        .lineHeight(22.0f)
                        .color(components::theme::withAlpha(colors.text, 0.72f))
                        .horizontalAlign(core::HorizontalAlign::Center)
                        .build();

                    components::input(ui, "fl_password_input")
                        .size(320.0f, 48.0f)
                        .text(s.password)
                        .placeholder("请输入密码（至少6个字符）")
                        .fontSize(16.0f)
                        .inset(14.0f)
                        .theme(colors)
                        .onChange([&](const std::string& value) {
                            s.password = value;
                        })
                        .onEnter([&] {
                            if (s.password.size() < 6) {
                                showToastMessage("错误", "密码长度至少需要6个字符");
                                return;
                            }
                            try {
                                diary::setPassword(s.password);
                                diary::saveSettings({static_cast<int>(s.themeSetting), static_cast<int>(s.backgroundEffect), s.userName});
                                showToastMessage("成功", "密码设置成功！");
                                s.diaryList = diary::listDiaryEntries(s.password);
                                goToPage(Page::MainMenu);
                            } catch (const std::exception& e) {
                                showToastMessage("错误", std::string("设置密码失败: ") + e.what());
                            }
                        })
                        .build();

                    components::button(ui, "fl_confirm_btn")
                        .size(240.0f, 48.0f)
                        .text("确认设置")
                        .fontSize(16.0f)
                        .theme(colors)
                        .onClick([&] {
                            if (s.password.size() < 6) {
                                showToastMessage("错误", "密码长度至少需要6个字符");
                                return;
                            }
                            try {
                                diary::setPassword(s.password);
                                diary::saveSettings({static_cast<int>(s.themeSetting), static_cast<int>(s.backgroundEffect), s.userName});
                                showToastMessage("成功", "密码设置成功！");
                                s.diaryList = diary::listDiaryEntries(s.password);
                                goToPage(Page::MainMenu);
                            } catch (const std::exception& e) {
                                showToastMessage("错误", std::string("设置密码失败: ") + e.what());
                            }
                        })
                        .build();
                });
        });
}

static void composeLogin(core::dsl::Ui& ui, const core::dsl::Screen& screen) {
    AppState& s = state();
    auto colors = currentThemeColors();
    auto panelGrad = getPanelGradient(colors);
    auto panelBorder = getPanelBorder(colors);

    ui.stack("login")
        .size(screen.width, screen.height)
        .align(core::Align::CENTER, core::Align::CENTER)
        .onTimer(s.loginVerifying ? 0.016f : 0.0f, [] {})
        .content([&] {
            components::panel(ui, "login_card")
                .size(420.0f, 320.0f)
                .radius(18.0f)
                .gradient(panelGrad.first, panelGrad.second)
                .border(1.0f, panelBorder)
                .shadow(26.0f, 0.0f, 8.0f, {0.0f, 0.0f, 0.0f, 0.26f})
                .build();

            ui.column("login_content")
                .size(420.0f, 320.0f)
                .gap(8.0f)
                .justifyContent(core::Align::CENTER)
                .alignItems(core::Align::CENTER)
                .content([&] {
                    components::text(ui, "login_title")
                        .size(360.0f, 36.0f)
                        .text("安全日记本")
                        .fontSize(26.0f)
                        .lineHeight(32.0f)
                        .color(components::theme::withAlpha(colors.text, 0.98f))
                        .horizontalAlign(core::HorizontalAlign::Center)
                        .build();

                    components::text(ui, "login_subtitle")
                        .size(360.0f, 24.0f)
                        .text(s.loginVerifying ? "正在验证身份…" : "请输入主密码解锁")
                        .fontSize(14.0f)
                        .lineHeight(20.0f)
                        .color(components::theme::withAlpha(colors.text, 0.72f))
                        .horizontalAlign(core::HorizontalAlign::Center)
                        .margin(0.0f, 0.0f, 0.0f, 4.0f)
                        .build();

                    auto now = std::chrono::steady_clock::now();
                    if (s.loginVerifying) {
                        float elapsed = std::chrono::duration<float>(now - s.loginStartTime).count();
                        s.loginProgress = std::min(elapsed / 4.0f, 0.88f);
                    }

                    ui.stack("login_action_area")
                        .size(320.0f, 100.0f)
                        .content([&] {
                            ui.column("login_normal_area")
                                .size(320.0f, 100.0f)
                                .gap(8.0f)
                                .justifyContent(core::Align::CENTER)
                                .alignItems(core::Align::CENTER)
                                .opacity(s.loginVerifying ? 0.0f : 1.0f)
                                .content([&] {
                                    components::input(ui, "login_password_input")
                                        .size(320.0f, 48.0f)
                                        .text(s.password)
                                        .placeholder("请输入密码")
                                        .fontSize(16.0f)
                                        .inset(14.0f)
                                        .theme(colors)
                                        .onChange([&](const std::string& value) {
                                            s.password = value;
                                        })
                                        .onEnter([&] {
                                            if (s.loginVerifying) return;
                                            s.loginVerifying = true;
                                            s.loginStartTime = std::chrono::steady_clock::now();
                                            s.loginProgress = 0.0f;
                                            g_loginResultReady.store(false, std::memory_order_relaxed);
                                            std::string pwd = s.password;
                                            s.loginThread = std::make_shared<std::thread>([pwd]() {
                                                bool result = diary::verifyPassword(pwd);
                                                g_loginSuccess.store(result, std::memory_order_relaxed);
                                                g_loginResultReady.store(true, std::memory_order_relaxed);
                                            });
                                        })
                                        .build();

                                    components::button(ui, "login_btn")
                                        .size(280.0f, 46.0f)
                                        .text("解锁")
                                        .fontSize(16.0f)
                                        .theme(colors)
                                        .onClick([&] {
                                            if (s.loginVerifying) return;
                                            s.loginVerifying = true;
                                            s.loginStartTime = std::chrono::steady_clock::now();
                                            s.loginProgress = 0.0f;
                                            g_loginResultReady.store(false, std::memory_order_relaxed);
                                            std::string pwd = s.password;
                                            s.loginThread = std::make_shared<std::thread>([pwd]() {
                                                bool result = diary::verifyPassword(pwd);
                                                g_loginSuccess.store(result, std::memory_order_relaxed);
                                                g_loginResultReady.store(true, std::memory_order_relaxed);
                                            });
                                        })
                                        .build();
                                })
                                .build();

                            ui.column("login_progress_area")
                                .size(320.0f, 100.0f)
                                .gap(10.0f)
                                .justifyContent(core::Align::CENTER)
                                .alignItems(core::Align::CENTER)
                                .opacity(s.loginVerifying ? 1.0f : 0.0f)
                                .content([&] {
                                    components::text(ui, "login_verify_label")
                                        .size(320.0f, 20.0f)
                                        .text("正在验证，请稍候...")
                                        .fontSize(14.0f)
                                        .lineHeight(18.0f)
                                        .color(components::theme::withAlpha(colors.text, 0.80f))
                                        .horizontalAlign(core::HorizontalAlign::Center)
                                        .build();

                                    components::progress(ui, "login_progress")
                                        .size(320.0f, 6.0f)
                                        .value(s.loginProgress)
                                        .theme(colors)
                                        .build();

                                    components::text(ui, "login_progress_text")
                                        .size(320.0f, 20.0f)
                                        .text(std::to_string(static_cast<int>(s.loginProgress * 100)) + "%")
                                        .fontSize(13.0f)
                                        .lineHeight(18.0f)
                                        .color(components::theme::withAlpha(colors.text, 0.55f))
                                        .horizontalAlign(core::HorizontalAlign::Center)
                                        .build();
                                })
                                .build();
                        })
                        .build();
                });
        });
}

static void composeMainMenu(core::dsl::Ui& ui, const core::dsl::Screen& screen) {
    AppState& s = state();
    auto colors = currentThemeColors();
    auto panelGrad = getPanelGradient(colors);
    auto panelBorder = getPanelBorder(colors);

    if (!s.welcomeShown && !s.userName.empty()) {
        s.welcomeShown = true;
        showToastMessage("欢迎回来", s.userName + "！");
    }

    ui.stack("main_menu")
        .size(screen.width, screen.height)
        .align(core::Align::CENTER, core::Align::CENTER)
        .content([&] {
            float sidebarWidth = 220.0f;
            float contentWidth = screen.width - sidebarWidth - 60.0f;
            float containerHeight = screen.height - 40.0f;
            float panelHeight = containerHeight;

            components::panel(ui, "menu_card")
                .size(screen.width - 40.0f, panelHeight)
                .radius(18.0f)
                .gradient(panelGrad.first, panelGrad.second)
                .border(1.0f, panelBorder)
                .shadow(26.0f, 0.0f, 8.0f, {0.0f, 0.0f, 0.0f, 0.26f})
                .build();

            ui.row("menu_layout")
                .size(screen.width - 40.0f, panelHeight)
                .gap(0.0f)
                .content([&] {
                    ui.column("sidebar")
                        .size(sidebarWidth, panelHeight)
                        .gap(8.0f)
                        .margin(20.0f, 20.0f, 20.0f, 20.0f)
                        .content([&] {
                            components::text(ui, "sidebar_title")
                                .size(sidebarWidth - 40.0f, 40.0f)
                                .text("安全日记本")
                                .fontSize(20.0f)
                                .lineHeight(26.0f)
                                .color(components::theme::withAlpha(colors.text, 0.98f))
                                .horizontalAlign(core::HorizontalAlign::Center)
                                .build();

                            ui.column("sidebar_buttons")
                                .size(sidebarWidth - 40.0f, panelHeight - 80.0f)
                                .gap(10.0f)
                                .content([&] {
                                    components::button(ui, "btn_new")
                                        .size(sidebarWidth - 40.0f, 44.0f)
                                        .text("新建日记")
                                        .fontSize(15.0f)
                                        .theme(colors)
                                        .onClick([&] {
                                            s.diaryContent.clear();
                                            s.mainMenuSelection = MainMenuSelection::New;
                                        })
                                        .build();

                                    components::button(ui, "btn_edit")
                                        .size(sidebarWidth - 40.0f, 44.0f)
                                        .text("编辑日记")
                                        .fontSize(15.0f)
                                        .theme(colors)
                                        .onClick([&] {
                                            s.diaryList = diary::listDiaryEntries(s.password);
                                            s.selectedDiaryIndex = -1;
                                            s.editListScrollOffset = 0.0f;
                                            s.mainMenuSelection = MainMenuSelection::Edit;
                                        })
                                        .build();

                                    components::button(ui, "btn_delete")
                                        .size(sidebarWidth - 40.0f, 44.0f)
                                        .text("删除日记")
                                        .fontSize(15.0f)
                                        .theme(colors)
                                        .onClick([&] {
                                            s.diaryList = diary::listDiaryEntries(s.password);
                                            s.selectedDiaryIndex = -1;
                                            s.showDeleteConfirm = false;
                                            s.deleteConfirmed = false;
                                            s.deleteListScrollOffset = 0.0f;
                                            s.mainMenuSelection = MainMenuSelection::Delete;
                                        })
                                        .build();

                                    components::button(ui, "btn_list")
                                        .size(sidebarWidth - 40.0f, 44.0f)
                                        .text("查看列表")
                                        .fontSize(15.0f)
                                        .theme(colors)
                                        .onClick([&] {
                                            s.diaryList = diary::listDiaryEntries(s.password);
                                            s.listScrollOffset = 0.0f;
                                            s.mainMenuSelection = MainMenuSelection::List;
                                        })
                                        .build();

                                    components::button(ui, "btn_settings")
                                        .size(sidebarWidth - 40.0f, 44.0f)
                                        .text("设置")
                                        .fontSize(15.0f)
                                        .secondaryTheme(colors)
                                        .onClick([&] {
                                            s.mainMenuSelection = MainMenuSelection::Settings;
                                        })
                                        .build();
                                })
                                .build();
                        })
                        .build();

                    ui.rect("divider")
                        .size(1.0f, panelHeight - 40.0f)
                        .color(components::theme::withAlpha(colors.text, 0.2f))
                        .build();

                    ui.column("content_pane")
                        .size(contentWidth, panelHeight)
                        .margin(20.0f, 20.0f, 20.0f, 20.0f)
                        .content([&] {
                            if (s.mainMenuSelection == MainMenuSelection::None) {
                                ui.stack("welcome_pane")
                                    .size(contentWidth, panelHeight - 40.0f)
                                    .align(core::Align::CENTER, core::Align::CENTER)
                                    .content([&] {
                                        components::text(ui, "welcome_text")
                                            .size(contentWidth - 40.0f, 60.0f)
                                            .text("请从左侧选择功能")
                                            .fontSize(24.0f)
                                            .lineHeight(30.0f)
                                            .color(components::theme::withAlpha(colors.text, 0.5f))
                                            .horizontalAlign(core::HorizontalAlign::Center)
                                            .build();
                                    })
                                    .build();
                            } else if (s.mainMenuSelection == MainMenuSelection::New) {
                                ui.column("new_pane")
                                    .size(contentWidth, panelHeight - 40.0f)
                                    .gap(12.0f)
                                    .justifyContent(core::Align::CENTER)
                                    .content([&] {
                                        components::text(ui, "new_title")
                                            .size(contentWidth - 40.0f, 32.0f)
                                            .text("新建日记")
                                            .fontSize(22.0f)
                                            .lineHeight(28.0f)
                                            .color(components::theme::withAlpha(colors.text, 0.98f))
                                            .horizontalAlign(core::HorizontalAlign::Center)
                                            .build();

                                        components::input(ui, "new_diary_input")
                                            .size(contentWidth - 80.0f, panelHeight - 200.0f)
                                            .text(s.diaryContent)
                                            .placeholder("在此输入日记内容...")
                                            .fontSize(15.0f)
                                            .inset(14.0f)
                                            .multiline(true)
                                            .theme(colors)
                                            .onChange([&](const std::string& value) {
                                                s.diaryContent = value;
                                            })
                                            .build();

                                        ui.row("new_buttons")
                                            .size(contentWidth - 40.0f, 48.0f)
                                            .gap(12.0f)
                                            .justifyContent(core::Align::CENTER)
                                            .alignItems(core::Align::CENTER)
                                            .content([&] {
                                                components::button(ui, "new_save_btn")
                                                    .size(180.0f, 48.0f)
                                                    .text("保存日记")
                                                    .fontSize(15.0f)
                                                    .theme(colors)
                                                    .onClick([&] {
                                                        if (s.diaryContent.empty()) {
                                                            showToastMessage("提示", "日记内容不能为空");
                                                            return;
                                                        }
                                                        try {
                                                            diary::createDiary(s.diaryContent, s.password);
                                                            s.diaryList = diary::listDiaryEntries(s.password);
                                                            s.diaryContent.clear();
                                                            showToastMessage("成功", "日记保存成功！");
                                                            s.mainMenuSelection = MainMenuSelection::List;
                                                        } catch (const std::exception& e) {
                                                            showToastMessage("错误", std::string("保存失败: ") + e.what());
                                                        }
                                                    })
                                                    .build();

                                                components::button(ui, "new_cancel_btn")
                                                    .size(180.0f, 48.0f)
                                                    .text("取消")
                                                    .fontSize(15.0f)
                                                    .secondaryTheme(colors)
                                                    .onClick([&] {
                                                        s.diaryContent.clear();
                                                        s.mainMenuSelection = MainMenuSelection::None;
                                                    })
                                                    .build();
                                            })
                                            .build();
                                    })
                                    .build();
                            } else if (s.mainMenuSelection == MainMenuSelection::Edit) {
                                if (s.selectedDiaryIndex == -1) {
                                    ui.column("edit_select_pane")
                                        .size(contentWidth, panelHeight - 40.0f)
                                        .gap(8.0f)
                                        .content([&] {
                                            components::text(ui, "edit_select_title")
                                                .size(contentWidth - 40.0f, 32.0f)
                                                .text("选择要编辑的日记")
                                                .fontSize(20.0f)
                                                .lineHeight(26.0f)
                                                .color(components::theme::withAlpha(colors.text, 0.98f))
                                                .horizontalAlign(core::HorizontalAlign::Center)
                                                .build();

                                            if (s.diaryList.empty()) {
                                                ui.stack("edit_empty_pane")
                                                    .size(contentWidth - 40.0f, 100.0f)
                                                    .align(core::Align::CENTER, core::Align::CENTER)
                                                    .content([&] {
                                                        components::text(ui, "edit_empty")
                                                            .size(contentWidth - 80.0f, 40.0f)
                                                            .text("还没有任何日记")
                                                            .fontSize(16.0f)
                                                            .lineHeight(22.0f)
                                                            .color(components::theme::withAlpha(colors.text, 0.62f))
                                                            .horizontalAlign(core::HorizontalAlign::Center)
                                                            .build();
                                                    })
                                                    .build();
                                            } else {
                                                float itemHeight = 46.0f;
                                                float totalContentHeight = static_cast<float>(s.diaryList.size()) * itemHeight;
                                                float viewportHeight = panelHeight - 160.0f;
                                                float scrollBarWidth = 10.0f;
                                                float contentAreaWidth = contentWidth - 40.0f - scrollBarWidth;

                                                ui.stack("edit_list_content_wrapper")
                                                    .size(contentWidth - 40.0f, viewportHeight)
                                                    .clip()
                                                    .onScroll([&, totalContentHeight, viewportHeight](const core::ScrollEvent& event) {
                                                        float maxScroll = std::max(0.0f, totalContentHeight - viewportHeight);
                                                        s.editListScrollOffset = std::clamp(s.editListScrollOffset - static_cast<float>(event.y) * 30.0f, 0.0f, maxScroll);
                                                    })
                                                    .content([&] {
                                                        for (size_t i = 0; i < s.diaryList.size(); ++i) {
                                                            const diary::DiaryEntry& entry = s.diaryList[i];
                                                            float itemY = static_cast<float>(i) * itemHeight - s.editListScrollOffset;

                                                            ui.row("edit_item_" + std::to_string(i))
                                                                .position(0.0f, itemY)
                                                                .size(contentAreaWidth, itemHeight)
                                                                .gap(12.0f)
                                                                .alignItems(core::Align::CENTER)
                                                                .content([&] {
                                                                    components::text(ui, "edit_item_time_" + std::to_string(i))
                                                                        .size(contentAreaWidth - 200.0f, 40.0f)
                                                                        .text(entry.folderName)
                                                                        .fontSize(14.0f)
                                                                        .lineHeight(18.0f)
                                                                        .color(components::theme::withAlpha(colors.text, 0.78f))
                                                                        .horizontalAlign(core::HorizontalAlign::Left)
                                                                        .verticalAlign(core::VerticalAlign::Center)
                                                                        .build();

                                                                    components::button(ui, "edit_select_btn_" + std::to_string(i))
                                                                        .size(120.0f, 36.0f)
                                                                        .text("选择")
                                                                        .fontSize(14.0f)
                                                                        .theme(colors)
                                                                        .onClick([&, i] {
                                                                            s.selectedDiaryIndex = static_cast<int>(i);
                                                                            std::string folderPath = std::string(diary::kLogDir) + "/" + s.diaryList[i].folderName;
                                                                            try {
                                                                                s.diaryContent = diary::readDiary(folderPath, s.password);
                                                                            } catch (const std::exception& e) {
                                                                                showToastMessage("错误", std::string("读取日记失败: ") + e.what());
                                                                            }
                                                                        })
                                                                        .build();
                                                                })
                                                                .build();
                                                        }

                                                        if (totalContentHeight > viewportHeight) {
                                                            components::scroll(ui, "edit_list_scroll")
                                                                .x(contentAreaWidth)
                                                                .size(scrollBarWidth, viewportHeight)
                                                                .viewport(viewportHeight)
                                                                .content(totalContentHeight)
                                                                .offset(s.editListScrollOffset)
                                                                .theme(colors)
                                                                .onChange([&](float value) {
                                                                    s.editListScrollOffset = value;
                                                                })
                                                                .build();
                                                        }
                                                    })
                                                    .build();
                                            }
                                        })
                                        .build();
                                } else {
                                    ui.column("edit_edit_pane")
                                        .size(contentWidth, panelHeight - 40.0f)
                                        .gap(12.0f)
                                        .justifyContent(core::Align::CENTER)
                                        .content([&] {
                                            std::string diaryName = s.diaryList[static_cast<size_t>(s.selectedDiaryIndex)].folderName;
                                            components::text(ui, "edit_title")
                                                .size(contentWidth - 40.0f, 32.0f)
                                                .text("编辑日记: " + diaryName)
                                                .fontSize(20.0f)
                                                .lineHeight(26.0f)
                                                .color(components::theme::withAlpha(colors.text, 0.98f))
                                                .horizontalAlign(core::HorizontalAlign::Center)
                                                .build();

                                            components::input(ui, "edit_diary_input")
                                                .size(contentWidth - 80.0f, panelHeight - 200.0f)
                                                .text(s.diaryContent)
                                                .placeholder("输入日记内容...")
                                                .fontSize(15.0f)
                                                .inset(14.0f)
                                                .multiline(true)
                                                .theme(colors)
                                                .onChange([&](const std::string& value) {
                                                    s.diaryContent = value;
                                                })
                                                .build();

                                            ui.row("edit_buttons")
                                                .size(contentWidth - 40.0f, 48.0f)
                                                .gap(12.0f)
                                                .justifyContent(core::Align::CENTER)
                                                .alignItems(core::Align::CENTER)
                                                .content([&] {
                                                    components::button(ui, "edit_save_btn")
                                                        .size(180.0f, 48.0f)
                                                        .text("保存修改")
                                                        .fontSize(15.0f)
                                                        .theme(colors)
                                                        .onClick([&] {
                                                            if (s.selectedDiaryIndex < 0 || s.selectedDiaryIndex >= static_cast<int>(s.diaryList.size())) {
                                                                showToastMessage("错误", "未选择有效的日记");
                                                                return;
                                                            }
                                                            if (s.diaryContent.empty()) {
                                                                showToastMessage("提示", "日记内容不能为空");
                                                                return;
                                                            }
                                                            try {
                                                                std::string folderPath = std::string(diary::kLogDir) + "/" + s.diaryList[static_cast<size_t>(s.selectedDiaryIndex)].folderName;
                                                                diary::editDiary(folderPath, s.diaryContent, s.password);
                                                                s.diaryList = diary::listDiaryEntries(s.password);
                                                                s.selectedDiaryIndex = -1;
                                                                showToastMessage("成功", "日记修改成功！");
                                                            } catch (const std::exception& e) {
                                                                showToastMessage("错误", std::string("保存失败: ") + e.what());
                                                            }
                                                        })
                                                        .build();

                                                    components::button(ui, "edit_cancel_btn")
                                                        .size(180.0f, 48.0f)
                                                        .text("取消")
                                                        .fontSize(15.0f)
                                                        .secondaryTheme(colors)
                                                        .onClick([&] {
                                                            s.selectedDiaryIndex = -1;
                                                        })
                                                        .build();
                                                })
                                                .build();
                                        })
                                        .build();
                                }
                            } else if (s.mainMenuSelection == MainMenuSelection::Delete) {
                                if (s.showDeleteConfirm && s.selectedDiaryIndex >= 0) {
                                    ui.column("delete_confirm_pane")
                                        .size(contentWidth, panelHeight - 40.0f)
                                        .gap(20.0f)
                                        .justifyContent(core::Align::CENTER)
                                        .alignItems(core::Align::CENTER)
                                        .content([&] {
                                            components::text(ui, "delete_confirm_title")
                                                .size(contentWidth - 40.0f, 40.0f)
                                                .text("确认删除？")
                                                .fontSize(20.0f)
                                                .lineHeight(26.0f)
                                                .color(components::theme::withAlpha(colors.text, 0.98f))
                                                .horizontalAlign(core::HorizontalAlign::Center)
                                                .build();

                                            ui.row("delete_confirm_buttons")
                                                .size(contentWidth - 40.0f, 48.0f)
                                                .gap(12.0f)
                                                .justifyContent(core::Align::CENTER)
                                                .alignItems(core::Align::CENTER)
                                                .content([&] {
                                                    components::button(ui, "delete_confirm_yes")
                                                        .size(180.0f, 48.0f)
                                                        .text("删除")
                                                        .fontSize(15.0f)
                                                        .colors(
                                                            core::Color{0.75f, 0.20f, 0.20f, 1.0f},
                                                            core::Color{0.85f, 0.25f, 0.25f, 1.0f},
                                                            core::Color{0.65f, 0.15f, 0.15f, 1.0f}
                                                        )
                                                        .onClick([&] {
                                                            if (s.selectedDiaryIndex < 0 || s.selectedDiaryIndex >= static_cast<int>(s.diaryList.size())) {
                                                                return;
                                                            }
                                                            try {
                                                                std::string folderPath = std::string(diary::kLogDir) + "/" + s.diaryList[static_cast<size_t>(s.selectedDiaryIndex)].folderName;
                                                                diary::deleteDiary(folderPath);
                                                                s.diaryList = diary::listDiaryEntries(s.password);
                                                                s.selectedDiaryIndex = -1;
                                                                s.showDeleteConfirm = false;
                                                                showToastMessage("成功", "日记删除成功！");
                                                            } catch (const std::exception& e) {
                                                                showToastMessage("错误", std::string("删除失败: ") + e.what());
                                                            }
                                                        })
                                                        .build();

                                                    components::button(ui, "delete_confirm_no")
                                                        .size(180.0f, 48.0f)
                                                        .text("取消")
                                                        .fontSize(15.0f)
                                                        .secondaryTheme(colors)
                                                        .onClick([&] {
                                                            s.showDeleteConfirm = false;
                                                        })
                                                        .build();
                                                })
                                                .build();
                                        })
                                        .build();
                                } else {
                                    ui.column("delete_select_pane")
                                        .size(contentWidth, panelHeight - 40.0f)
                                        .gap(8.0f)
                                        .content([&] {
                                            components::text(ui, "delete_title")
                                                .size(contentWidth - 40.0f, 32.0f)
                                                .text("删除日记")
                                                .fontSize(20.0f)
                                                .lineHeight(26.0f)
                                                .color(components::theme::withAlpha(colors.text, 0.98f))
                                                .horizontalAlign(core::HorizontalAlign::Center)
                                                .build();

                                            if (s.diaryList.empty()) {
                                                ui.stack("delete_empty_pane")
                                                    .size(contentWidth - 40.0f, 100.0f)
                                                    .align(core::Align::CENTER, core::Align::CENTER)
                                                    .content([&] {
                                                        components::text(ui, "delete_empty")
                                                            .size(contentWidth - 80.0f, 40.0f)
                                                            .text("还没有任何日记")
                                                            .fontSize(16.0f)
                                                            .lineHeight(22.0f)
                                                            .color(components::theme::withAlpha(colors.text, 0.62f))
                                                            .horizontalAlign(core::HorizontalAlign::Center)
                                                            .build();
                                                    })
                                                    .build();
                                            } else {
                                                float itemHeight = 46.0f;
                                                float totalContentHeight = static_cast<float>(s.diaryList.size()) * itemHeight;
                                                float viewportHeight = panelHeight - 160.0f;
                                                float scrollBarWidth = 10.0f;
                                                float contentAreaWidth = contentWidth - 40.0f - scrollBarWidth;

                                                ui.stack("delete_list_content_wrapper")
                                                    .size(contentWidth - 40.0f, viewportHeight)
                                                    .clip()
                                                    .onScroll([&, totalContentHeight, viewportHeight](const core::ScrollEvent& event) {
                                                        float maxScroll = std::max(0.0f, totalContentHeight - viewportHeight);
                                                        s.deleteListScrollOffset = std::clamp(s.deleteListScrollOffset - static_cast<float>(event.y) * 30.0f, 0.0f, maxScroll);
                                                    })
                                                    .content([&] {
                                                        for (size_t i = 0; i < s.diaryList.size(); ++i) {
                                                            const diary::DiaryEntry& entry = s.diaryList[i];
                                                            float itemY = static_cast<float>(i) * itemHeight - s.deleteListScrollOffset;

                                                            ui.row("delete_item_" + std::to_string(i))
                                                                .position(0.0f, itemY)
                                                                .size(contentAreaWidth, itemHeight)
                                                                .gap(12.0f)
                                                                .alignItems(core::Align::CENTER)
                                                                .content([&] {
                                                                    components::text(ui, "delete_item_time_" + std::to_string(i))
                                                                        .size(contentAreaWidth - 200.0f, 40.0f)
                                                                        .text(entry.folderName)
                                                                        .fontSize(14.0f)
                                                                        .lineHeight(18.0f)
                                                                        .color(components::theme::withAlpha(colors.text, 0.78f))
                                                                        .horizontalAlign(core::HorizontalAlign::Left)
                                                                        .verticalAlign(core::VerticalAlign::Center)
                                                                        .build();

                                                                    components::button(ui, "delete_btn_" + std::to_string(i))
                                                                        .size(120.0f, 36.0f)
                                                                        .text("删除")
                                                                        .fontSize(14.0f)
                                                                        .theme(colors)
                                                                        .colors(
                                                                            core::Color{0.75f, 0.20f, 0.20f, 1.0f},
                                                                            core::Color{0.85f, 0.25f, 0.25f, 1.0f},
                                                                            core::Color{0.65f, 0.15f, 0.15f, 1.0f}
                                                                        )
                                                                        .onClick([&, i] {
                                                                            s.selectedDiaryIndex = static_cast<int>(i);
                                                                            s.showDeleteConfirm = true;
                                                                        })
                                                                        .build();
                                                                })
                                                                .build();
                                                        }

                                                        if (totalContentHeight > viewportHeight) {
                                                            components::scroll(ui, "delete_list_scroll")
                                                                .x(contentAreaWidth)
                                                                .size(scrollBarWidth, viewportHeight)
                                                                .viewport(viewportHeight)
                                                                .content(totalContentHeight)
                                                                .offset(s.deleteListScrollOffset)
                                                                .theme(colors)
                                                                .onChange([&](float value) {
                                                                    s.deleteListScrollOffset = value;
                                                                })
                                                                .build();
                                                        }
                                                    })
                                                    .build();
                                            }
                                        })
                                        .build();
                                }
                            } else if (s.mainMenuSelection == MainMenuSelection::List) {
                                ui.column("list_pane")
                                    .size(contentWidth, panelHeight - 40.0f)
                                    .gap(8.0f)
                                    .content([&] {
                                        components::text(ui, "list_title")
                                            .size(contentWidth - 40.0f, 32.0f)
                                            .text("日记列表 (" + std::to_string(s.diaryList.size()) + " 篇)")
                                            .fontSize(20.0f)
                                            .lineHeight(26.0f)
                                            .color(components::theme::withAlpha(colors.text, 0.98f))
                                            .horizontalAlign(core::HorizontalAlign::Center)
                                            .build();

                                        if (s.diaryList.empty()) {
                                            ui.stack("list_empty_pane")
                                                .size(contentWidth - 40.0f, 100.0f)
                                                .align(core::Align::CENTER, core::Align::CENTER)
                                                .content([&] {
                                                    components::text(ui, "list_empty")
                                                        .size(contentWidth - 80.0f, 40.0f)
                                                        .text("还没有任何日记")
                                                        .fontSize(16.0f)
                                                        .lineHeight(22.0f)
                                                        .color(components::theme::withAlpha(colors.text, 0.62f))
                                                        .horizontalAlign(core::HorizontalAlign::Center)
                                                        .build();
                                                })
                                                .build();
                                        } else {
                                            std::vector<float> itemHeights;
                                            float headerH = 36.0f;
                                            float lineH = 18.0f;
                                            float bottomPad = 8.0f;
                                            float charsPerLine = (contentWidth - 40.0f - 10.0f) / (13.0f * 0.55f);
                                            for (const auto& entry : s.diaryList) {
                                                int blankLines = 0;
                                                for (char c : entry.preview) { if (c == '\n') ++blankLines; }
                                                float textLines = std::ceil(static_cast<float>(entry.preview.size()) / charsPerLine) + static_cast<float>(blankLines);
                                                float textH = std::max(textLines * lineH, lineH);
                                                itemHeights.push_back(headerH + textH + bottomPad);
                                            }
                                            float totalContentHeight = 0.0f;
                                            for (float h : itemHeights) totalContentHeight += h;

                                            float viewportHeight = panelHeight - 120.0f;
                                            float scrollBarWidth = 10.0f;
                                            float contentAreaWidth = contentWidth - 40.0f - scrollBarWidth;

                                            ui.stack("list_content_wrapper")
                                                .size(contentWidth - 40.0f, viewportHeight)
                                                .clip()
                                                .onScroll([&, totalContentHeight, viewportHeight](const core::ScrollEvent& event) {
                                                    float maxScroll = std::max(0.0f, totalContentHeight - viewportHeight);
                                                    s.listScrollOffset = std::clamp(s.listScrollOffset - static_cast<float>(event.y) * 30.0f, 0.0f, maxScroll);
                                                })
                                                .content([&] {
                                                    float runningY = 0.0f;
                                                    for (size_t i = 0; i < s.diaryList.size(); ++i) {
                                                        const diary::DiaryEntry& entry = s.diaryList[i];
                                                        float itemH = itemHeights[i];
                                                        float itemY = runningY - s.listScrollOffset;

                                                        ui.column("list_item_" + std::to_string(i))
                                                            .position(0.0f, itemY)
                                                            .size(contentAreaWidth, itemH)
                                                            .content([&] {
                                                                components::text(ui, "list_item_time_" + std::to_string(i))
                                                                    .position(0.0f, 8.0f)
                                                                    .size(contentAreaWidth, 24.0f)
                                                                    .text(entry.folderName)
                                                                    .fontSize(14.0f)
                                                                    .lineHeight(20.0f)
                                                                    .color(components::theme::withAlpha(colors.primary, 0.88f))
                                                                    .horizontalAlign(core::HorizontalAlign::Left)
                                                                    .build();

                                                                components::text(ui, "list_item_content_" + std::to_string(i))
                                                                    .position(0.0f, headerH)
                                                                    .size(contentAreaWidth, itemH - headerH)
                                                                    .text(entry.preview)
                                                                    .fontSize(13.0f)
                                                                    .lineHeight(lineH)
                                                                    .color(components::theme::withAlpha(colors.text, 0.68f))
                                                                    .horizontalAlign(core::HorizontalAlign::Left)
                                                                    .build();
                                                            })
                                                            .build();

                                                        runningY += itemH;
                                                    }

                                                    if (totalContentHeight > viewportHeight) {
                                                        components::scroll(ui, "list_scroll")
                                                            .x(contentAreaWidth)
                                                            .size(scrollBarWidth, viewportHeight)
                                                            .viewport(viewportHeight)
                                                            .content(totalContentHeight)
                                                            .offset(s.listScrollOffset)
                                                            .theme(colors)
                                                            .onChange([&](float value) {
                                                                s.listScrollOffset = value;
                                                            })
                                                            .build();
                                                    }
                                                })
                                                .build();
                                        }
                                    })
                                    .build();
                            } else if (s.mainMenuSelection == MainMenuSelection::Settings) {
                                ui.column("settings_pane")
                                    .size(contentWidth - 80.0f, panelHeight - 40.0f)
                                    .gap(18.0f)
                                    .justifyContent(core::Align::CENTER)
                                    .alignItems(core::Align::CENTER)
                                    .content([&] {
                                        components::text(ui, "settings_title")
                                            .size(contentWidth - 160.0f, 36.0f)
                                            .text("设置")
                                            .fontSize(22.0f)
                                            .lineHeight(28.0f)
                                            .color(components::theme::withAlpha(colors.text, 0.98f))
                                            .horizontalAlign(core::HorizontalAlign::Center)
                                            .margin(0.0f, 0.0f, 0.0f, 12.0f)
                                            .build();

                                        ui.column("name_row")
                                            .size(contentWidth - 160.0f, 90.0f)
                                            .gap(8.0f)
                                            .justifyContent(core::Align::CENTER)
                                            .alignItems(core::Align::CENTER)
                                            .content([&] {
                                                components::text(ui, "settings_name_label")
                                                    .size(contentWidth - 160.0f, 26.0f)
                                                    .text("称呼")
                                                    .fontSize(16.0f)
                                                    .lineHeight(20.0f)
                                                    .color(components::theme::withAlpha(colors.text, 0.78f))
                                                    .horizontalAlign(core::HorizontalAlign::Left)
                                                    .build();
                                                components::input(ui, "settings_name_input")
                                                    .size(contentWidth - 160.0f, 48.0f)
                                                    .text(s.userName)
                                                    .placeholder("请输入您的称呼")
                                                    .fontSize(16.0f)
                                                    .inset(12.0f)
                                                    .theme(colors)
                                                    .onChange([&](const std::string& value) {
                                                        s.userName = value;
                                                    })
                                                    .onEnter([&] {
                                                        diary::saveSettings({static_cast<int>(s.themeSetting), static_cast<int>(s.backgroundEffect), s.userName});
                                                        showToastMessage("已保存", "称呼已更新");
                                                    })
                                                    .build();
                                            })
                                            .build();

                                        ui.column("theme_row")
                                            .size(contentWidth - 160.0f, 72.0f)
                                            .gap(8.0f)
                                            .justifyContent(core::Align::CENTER)
                                            .alignItems(core::Align::CENTER)
                                            .content([&] {
                                                components::text(ui, "theme_label")
                                                    .size(contentWidth - 160.0f, 26.0f)
                                                    .text("主题")
                                                    .fontSize(16.0f)
                                                    .lineHeight(20.0f)
                                                    .color(components::theme::withAlpha(colors.text, 0.78f))
                                                    .horizontalAlign(core::HorizontalAlign::Left)
                                                    .build();
                                                components::segmented(ui, "theme_switch")
                                                    .size(contentWidth - 160.0f, 44.0f)
                                                    .items({"暗色", "亮色"})
                                                    .selected(static_cast<int>(s.themeSetting))
                                                    .fontSize(16.0f)
                                                    .theme(colors)
                                                    .onChange([&](int index) {
                                                        s.themeSetting = static_cast<ThemeSetting>(index);
                                                        diary::saveSettings({static_cast<int>(s.themeSetting), static_cast<int>(s.backgroundEffect), s.userName});
                                                    })
                                                    .build();
                                            })
                                            .build();

                                        ui.column("effect_row")
                                            .size(contentWidth - 160.0f, 72.0f)
                                            .gap(8.0f)
                                            .justifyContent(core::Align::CENTER)
                                            .alignItems(core::Align::CENTER)
                                            .content([&] {
                                                components::text(ui, "effect_label")
                                                    .size(contentWidth - 160.0f, 26.0f)
                                                    .text("背景效果")
                                                    .fontSize(16.0f)
                                                    .lineHeight(20.0f)
                                                    .color(components::theme::withAlpha(colors.text, 0.78f))
                                                    .horizontalAlign(core::HorizontalAlign::Left)
                                                    .build();
                                                components::segmented(ui, "effect_switch")
                                                    .size(contentWidth - 160.0f, 44.0f)
                                                    .items({"亚克力", "毛玻璃"})
                                                    .selected(static_cast<int>(s.backgroundEffect))
                                                    .fontSize(16.0f)
                                                    .theme(colors)
                                                    .onChange([&](int index) {
                                                        s.backgroundEffect = static_cast<BackgroundEffect>(index);
                                                        diary::saveSettings({static_cast<int>(s.themeSetting), static_cast<int>(s.backgroundEffect), s.userName});
                                                    })
                                                    .build();
                                            })
                                            .build();
                                    })
                                    .build();
                            }
                        })
                        .build();
                })
                .build();
        });
}

void compose(core::dsl::Ui& ui, const core::dsl::Screen& screen) {
    AppState& s = state();

    if (!s.initialized) {
        diary::ensureLogDirectoryExists();

        diary::Settings settings = diary::loadSettings();
        if (settings.theme >= 0) {
            s.themeSetting = static_cast<ThemeSetting>(settings.theme);
        }
        if (settings.effect >= 0) {
            s.backgroundEffect = static_cast<BackgroundEffect>(settings.effect);
        }
        if (!settings.name.empty()) {
            s.userName = settings.name;
        }

        if (diary::isFirstLaunch()) {
            s.currentPage = Page::FirstLaunchSetPassword;
        } else {
            s.currentPage = Page::Login;
        }
        s.initialized = true;
    }

    auto colors = currentThemeColors();

    ui.stack("app_bg")
        .size(screen.width, screen.height)
        .content([&] {
            ui.rect("bg_rect")
                .size(screen.width, screen.height)
                .color(colors.background)
                .build();
        })
        .build();

    if (s.loginVerifying && g_loginResultReady.load(std::memory_order_relaxed)) {
        bool success = g_loginSuccess.load(std::memory_order_relaxed);
        g_loginResultReady.store(false, std::memory_order_relaxed);
        s.loginVerifying = false;
        if (s.loginThread) {
            if (s.loginThread->joinable()) {
                s.loginThread->join();
            }
            s.loginThread.reset();
        }
        if (success) {
            s.diaryList = diary::listDiaryEntries(s.password);
            goToPage(Page::MainMenu);
        } else {
            showToastMessage("密码错误", "您输入的密码不正确，请重试");
        }
    }

    switch (s.currentPage) {
        case Page::FirstLaunchSetPassword:
            composeFirstLaunchSetPassword(ui, screen);
            break;
        case Page::Login:
            composeLogin(ui, screen);
            break;
        case Page::MainMenu:
            composeMainMenu(ui, screen);
            break;
    }

    components::toast(ui, "global_toast")
        .visible(s.showToast)
        .screen(screen.width, screen.height)
        .size(360.0f, 88.0f)
        .title(s.toastTitle)
        .message(s.toastMessage)
        .transition(core::Transition::make(0.3f, core::Ease::OutCubic))
        .autoDismiss(3.0f)
        .onAutoDismiss([&] {
            s.showToast = false;
        })
        .onDismiss([&] {
            s.showToast = false;
        })
        .build();
}

}
