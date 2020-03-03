<template>
    <div class="404">
        <md-empty-state
            class="md-accent"
            md-icon="error_outline"
            md-label="Страница не найдена"
            md-description="Данная страница не найдена. Попробуйте ввести другой запрос.">
            <md-button class="md-raised" @click="$router.push('/')">Назад</md-button>
        </md-empty-state>
    </div>
</template>

<script>
    import store from '../store';

    export default {
        beforeRouteEnter: function (to, from, next) {
            if (store.state.auth.token) {
                next();
            } else {
                next('/');
            }
        }
    }
</script>
