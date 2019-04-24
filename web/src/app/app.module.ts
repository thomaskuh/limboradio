import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';

import { AppComponent } from './app.component';
import {
  MatButtonModule,
  MatCardModule, MatDialogModule,
  MatFormFieldModule,
  MatIconModule,
  MatInputModule,
  MatListModule, MatProgressBarModule, MatSelectModule,
  MatSliderModule, MatSnackBarModule,
  MatToolbarModule
} from '@angular/material';
import {HttpClient, HttpClientModule} from '@angular/common/http';
import {BrowserAnimationsModule} from '@angular/platform-browser/animations';
import { CardStatusComponent } from './card-status/card-status.component';
import {FormsModule} from '@angular/forms';
import {TranslateLoader, TranslateModule} from '@ngx-translate/core';
import {TranslateHttpLoader} from '@ngx-translate/http-loader';
import { CardErrorComponent } from './card-error/card-error.component';
import { CardConfigComponent } from './card-config/card-config.component';
import { CardStreamsComponent } from './card-streams/card-streams.component';
import { CardWifiComponent } from './card-wifi/card-wifi.component';
import { DialogErrorComponent } from './dialog-error/dialog-error.component';
import { DialogStreamComponent } from './dialog-stream/dialog-stream.component';
import { SnackbarMessageComponent } from './snackbar-message/snackbar-message.component';

export function createTranslateLoader(http: HttpClient) {
  return new TranslateHttpLoader(http, './assets/i18n/', '.json');
}

@NgModule({
  declarations: [
    AppComponent,
    CardStatusComponent,
    CardErrorComponent,
    CardConfigComponent,
    CardStreamsComponent,
    CardWifiComponent,
    DialogErrorComponent,
    DialogStreamComponent,
    SnackbarMessageComponent
  ],
  imports: [
    BrowserModule,
    BrowserAnimationsModule,
    HttpClientModule,
    FormsModule,
    MatToolbarModule,
    MatIconModule,
    MatCardModule,
    MatFormFieldModule,
    MatInputModule,
    MatSliderModule,
    MatProgressBarModule,
    MatListModule,
    MatSelectModule,
    MatButtonModule,
    MatDialogModule,
    MatSnackBarModule,
    TranslateModule.forRoot({
      loader: {
        provide: TranslateLoader,
        useFactory: (createTranslateLoader),
        deps: [HttpClient]
      }
    })
  ],
  entryComponents: [
    DialogErrorComponent,
    DialogStreamComponent,
    SnackbarMessageComponent
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
